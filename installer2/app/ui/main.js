const { invoke } = window.__TAURI__.core;

invoke('core_version').then((version) => {
  document.getElementById('core-version').textContent = version;
});

invoke('payload_info').then(
  (info) => { document.getElementById('payload-info').textContent = info; },
  (error) => { document.getElementById('payload-info').textContent = `error: ${error}`; },
);
