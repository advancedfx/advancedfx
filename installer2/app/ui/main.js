// HLAE Setup wizard. All texts come from the translation catalog (installer2/l10n/messages.pot).
const { invoke } = window.__TAURI__.core;
const { listen } = window.__TAURI__.event;
const $ = (id) => document.getElementById(id);

let catalog = {};
let info = null;
/** 'install' | 'uninstall' while / after running */
let operation = null;
/** page to go back to from install (modify) / uninstall */
let backPage = null;

/** Text for `key`, `{name}` placeholders replaced; the key itself if unknown (visible bug). */
function t(key, args = {}) {
  const text = catalog[key] ?? key;
  return text.replace(/\{(\w+)\}/g, (match, name) => (name in args ? String(args[name]) : match));
}

function applyCatalog() {
  for (const el of document.querySelectorAll('[data-l10n]')) el.textContent = t(el.dataset.l10n);
  document.documentElement.lang = info.language;
  document.title = t('WindowTitle');
  $('header-version').textContent = t('HeaderVersion', { version: info.version });
}

/** id of the visible page */
let currentPage = 'page-loading';

function show(id) {
  currentPage = id;
  for (const page of document.querySelectorAll('.page')) page.hidden = page.id !== id;
  $(id).querySelector('h1')?.focus();
}

const quit = () => invoke('quit');

// ---- install / modify page ----

const variantLabel = (v) => t(`FfmpegVariant_${v.linking}_${v.license}`, { size: v.sizeMb });
const ffmpegKind = () => document.querySelector('input[name="ffmpeg"]:checked')?.value;

function installedFfmpegText(installed) {
  if (installed.ffmpegChoice === 'custom') return installed.ffmpegCustom ?? '';
  const variant = info.variants.find((v) => v.id === installed.ffmpegVariant);
  return variant ? t(`FfmpegVariant_${variant.linking}_${variant.license}`, { size: variant.sizeMb }) : installed.ffmpegVariant ?? '';
}

function setError(field, key) {
  const input = $(field);
  const error = $(`${field}-error`);
  input.setAttribute('aria-invalid', key ? 'true' : 'false');
  error.hidden = !key;
  error.textContent = key ? t(key) : '';
}

let validation = 0;
async function validate() {
  const run = ++validation;
  const dirError = info.installed ? null : await invoke('check_install_dir', { path: $('install-dir').value.trim() });
  const exeError = ffmpegKind() === 'custom'
    ? await invoke('check_ffmpeg_exe', { path: $('ffmpeg-exe').value.trim() })
    : null;
  if (run !== validation) return false; // a newer validation is running
  setError('install-dir', dirError);
  setError('ffmpeg-exe', exeError);
  const ok = !dirError && !exeError && info.hasPayload && !info.legacy?.wixV3;
  $('install-go').disabled = !ok;
  return ok;
}

function updateFfmpegOptions() {
  for (const sub of document.querySelectorAll('.sub')) sub.hidden = sub.dataset.for !== ffmpegKind();
  validate();
}

function openInstall(modify) {
  const installed = info.installed;
  $('install-heading').textContent = modify ? t('ModifyHeading') : t('InstallHeading', { version: info.version });
  $('install-dir').value = installed ? installed.installDir : info.defaultInstallDir;
  $('install-dir').disabled = $('browse-dir').disabled = !!installed;
  $('install-dir-fixed').hidden = !installed;
  $('locales').checked = installed ? installed.locales : true;

  const select = $('ffmpeg-variant');
  select.replaceChildren(...info.variants.map((v) => new Option(variantLabel(v), v.id)));
  const hasFfmpeg = !!installed && installed.ffmpegChoice !== 'none';
  $('ffmpeg-keep-option').hidden = !hasFfmpeg;
  if (hasFfmpeg) $('ffmpeg-keep-label').textContent = t('FfmpegKeep', { what: installedFfmpegText(installed) });
  $('ffmpeg-none-label').textContent = t(hasFfmpeg ? 'FfmpegRemove' : 'FfmpegNone');
  $('ffmpeg-exe').value = installed?.ffmpegCustom ?? '';

  // Preselect the defaults (NOTES.md 4.2).
  const preset = info.defaultFfmpeg;
  let kind = preset.kind === 'keep' && !hasFfmpeg ? 'none' : preset.kind;
  select.value = preset.kind === 'download' ? preset.variant : installed?.ffmpegVariant ?? info.variants[0]?.id;
  if (preset.kind === 'custom') $('ffmpeg-exe').value = preset.exe;
  document.querySelector(`input[name="ffmpeg"][value="${kind}"]`).checked = true;

  $('no-payload').hidden = info.hasPayload;
  const legacy = installed ? null : info.legacy;
  $('legacy-notice').hidden = !legacy;
  if (legacy) {
    $('legacy-notice').textContent = t(legacy.wixV3 ? 'LegacyWixV3' : 'LegacyReplace', { installed: legacy.version, version: info.version });
  }
  $('dotnet-missing').hidden = info.dotnetOk;
  $('dotnet-missing').textContent = t('DotnetMissing', { version: info.dotnetVersion });
  $('install-go').textContent = t(modify ? 'ApplyButton' : 'InstallButton');
  $('install-back').hidden = !backPage;
  show('page-install');
  updateFfmpegOptions();
}

function selectedFfmpeg() {
  const kind = ffmpegKind();
  if (kind === 'download') return { kind, variant: $('ffmpeg-variant').value };
  if (kind === 'custom') return { kind, exe: $('ffmpeg-exe').value.trim() };
  return { kind };
}

// ---- maintenance / uninstall pages ----

function openMaintenance() {
  const installed = info.installed;
  $('maintenance-heading').textContent = t('MaintenanceHeading', { version: installed.version });
  $('maintenance-location').textContent = t('MaintenanceLocation', { dir: installed.installDir });
  $('maintenance-setup').textContent = t('MaintenanceSetup', { version: info.version });
  const newer = info.newerInstalled;
  $('maintenance-newer').hidden = !newer;
  $('maintenance-newer').textContent = t('NewerInstalled', { installed: installed.version, version: info.version });
  $('maintenance-update').hidden = $('maintenance-modify').hidden = newer;
  $('maintenance-update').textContent = installed.version === info.version
    ? t('RepairButton')
    : t('UpgradeButton', { version: info.version });
  show('page-maintenance');
}

function openUninstall() {
  $('uninstall-heading').textContent = t('UninstallHeading', { version: info.installed.version });
  $('uninstall-back').hidden = !backPage;
  show('page-uninstall');
}

// ---- files in use ----

/** What to run once no program uses the files any more, and where Back goes. */
let inUse = null;
/** A check is running (Retry isn't disabled for that: it would lose the keyboard focus). */
let checkingInUse = false;

/** Runs `run` if no program uses files the operation touches, else lists them with Retry. */
async function whenNotInUse(kind, run) {
  if (checkingInUse) return;
  const retry = inUse !== null && currentPage === 'page-in-use';
  if (!retry) inUse = { kind, run, from: currentPage };
  checkingInUse = true;
  let programs = [];
  try {
    programs = await invoke('programs_in_use', { install: kind === 'install' });
  } finally {
    checkingInUse = false;
  }
  if (programs.length === 0) {
    inUse = null;
    return run();
  }
  $('in-use-list').replaceChildren(...programs.map((p) => {
    const item = document.createElement('li');
    item.textContent = p.name;
    if (p.exe && p.exe.toLowerCase() !== p.name.toLowerCase()) {
      const exe = document.createElement('span');
      exe.className = 'exe';
      exe.textContent = ` – ${p.exe}`;
      item.append(exe);
    }
    return item;
  }));
  $('in-use-still').hidden = !retry;
  if (!retry) show('page-in-use');
}

// ---- running ----

function startProgress(kind) {
  operation = kind;
  $('progress-heading').textContent = t(kind === 'install' ? 'ProgressInstalling' : 'ProgressUninstalling');
  $('log').textContent = '';
  setPhase(null);
  show('page-progress');
}

const startInstall = (installDir, locales, ffmpeg) => whenNotInUse('install', async () => {
  startProgress('install');
  try {
    await invoke('start_install', { installDir, locales, ffmpeg });
  } catch (e) {
    appendLog('ERROR', String(e));
    finish(1603);
  }
});

const startUninstall = () => whenNotInUse('uninstall', async () => {
  startProgress('uninstall');
  try {
    await invoke('start_uninstall', { removeAppData: $('remove-app-data').checked });
  } catch (e) {
    appendLog('ERROR', String(e));
    finish(1603);
  }
});

const mb = (bytes) => Math.round(bytes / 1_000_000);

function setPhase(phase) {
  const bar = $('progress-bar');
  $('progress-cancel').hidden = phase !== 'download';
  $('progress-detail').textContent = '';
  let status = '';
  if (phase === 'download') {
    status = t('PhaseDownload');
    bar.value = 0;
  } else {
    bar.removeAttribute('value'); // indeterminate
    if (phase === 'elevate') status = t('PhaseElevate');
    if (phase === 'work') status = t(operation === 'install' ? 'PhaseWorkInstall' : 'PhaseWorkUninstall');
  }
  $('progress-status').textContent = status;
}

function appendLog(level, message) {
  const log = $('log');
  const atEnd = log.scrollTop + log.clientHeight >= log.scrollHeight - 4;
  log.textContent += `${level.padEnd(5)} ${message}\n`;
  if (atEnd) log.scrollTop = log.scrollHeight;
}

function finish(code) {
  const known = {
    0: ['✓', operation === 'install' ? t('DoneInstalled', { version: info.version }) : t('DoneUninstalled')],
    1602: ['!', t('DoneCancelled')],
    1223: ['!', t('DoneDeclined')],
    1605: ['!', t('DoneNotInstalled')],
  };
  const [icon, heading] = known[code] ?? ['✕', t('DoneFailed')];
  $('done-icon').textContent = icon;
  $('done-icon').dataset.result = code === 0 ? 'ok' : code in known ? 'info' : 'error';
  $('done-heading').textContent = heading;
  $('done-text').textContent = code in known ? '' : t('DoneFailedText');
  $('done-launch').hidden = !(code === 0 && operation === 'install');
  show('page-done');
}

// ---- wiring ----

function wire() {
  $('install-dir').addEventListener('input', validate);
  $('ffmpeg-exe').addEventListener('input', validate);
  for (const radio of document.querySelectorAll('input[name="ffmpeg"]')) radio.addEventListener('change', updateFfmpegOptions);

  $('browse-dir').addEventListener('click', async () => {
    let dir = await invoke('browse_folder', { initial: $('install-dir').value });
    if (!dir) return;
    // Like most setups: install into an "HLAE" subfolder of what was picked.
    if (!/[\\/]HLAE$/i.test(dir)) dir = `${dir.replace(/[\\/]+$/, '')}\\HLAE`;
    $('install-dir').value = dir;
    validate();
  });
  $('browse-exe').addEventListener('click', async () => {
    const exe = await invoke('browse_ffmpeg_exe', { initial: $('ffmpeg-exe').value });
    if (exe) {
      $('ffmpeg-exe').value = exe;
      validate();
    }
  });

  $('install-go').addEventListener('click', async () => {
    if (await validate()) startInstall($('install-dir').value.trim(), $('locales').checked, selectedFfmpeg());
  });
  $('maintenance-update').addEventListener('click', () => {
    const installed = info.installed;
    startInstall(installed.installDir, installed.locales, info.defaultFfmpeg);
  });
  $('maintenance-modify').addEventListener('click', () => {
    backPage = 'page-maintenance';
    openInstall(true);
  });
  $('maintenance-uninstall').addEventListener('click', () => {
    backPage = 'page-maintenance';
    openUninstall();
  });
  $('uninstall-go').addEventListener('click', startUninstall);
  for (const id of ['install-back', 'uninstall-back']) $(id).addEventListener('click', () => show(backPage));
  $('in-use-retry').addEventListener('click', () => whenNotInUse(inUse.kind, inUse.run));
  $('in-use-back').addEventListener('click', () => {
    const from = inUse.from;
    inUse = null;
    show(from);
  });
  for (const id of ['install-cancel', 'maintenance-close', 'uninstall-cancel', 'done-close']) $(id).addEventListener('click', quit);
  $('progress-cancel').addEventListener('click', () => invoke('cancel'));
  $('done-log').addEventListener('click', () => invoke('open_log'));
  $('done-launch').addEventListener('click', async () => {
    await invoke('launch_hlae').catch((e) => appendLog('ERROR', String(e)));
    quit();
  });

  listen('setup-event', ({ payload: event }) => {
    if (event.kind === 'log') appendLog(event.level, event.message);
    if (event.kind === 'phase') setPhase(event.phase);
    if (event.kind === 'download') {
      $('progress-bar').value = event.total ? event.done / event.total : 0;
      $('progress-detail').textContent = t('PhaseDownloadProgress', { done: mb(event.done), total: mb(event.total) });
    }
  });
  listen('setup-finished', ({ payload: code }) => finish(code));
}

async function start() {
  [catalog, info] = await Promise.all([invoke('l10n_catalog'), invoke('info')]);
  applyCatalog();
  wire();
  const installed = info.installed;
  if (!installed) {
    if (info.mode === 'uninstall') {
      operation = 'uninstall';
      return finish(1605);
    }
    return openInstall(false);
  }
  if (info.mode === 'uninstall') return openUninstall();
  if (info.mode === 'modify' && !info.newerInstalled) return openInstall(true);
  openMaintenance();
}

start().catch((e) => {
  document.body.textContent = `HLAE Setup: ${e}`;
});
