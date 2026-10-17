//! Translation catalogs for the GUI (NOTES.md 6.9): the English source `.pot` and one `.po` per
//! language become `msgctxt → text` maps, which `app/build.rs` embeds as JSON. Parsing via `polib`.

use std::collections::{BTreeMap, BTreeSet};
use std::io::Read;

/// `msgctxt` (stable key) → text.
pub type Catalog = BTreeMap<String, String>;

/// English catalog from the `.pot`: `msgctxt → msgid`. Every message needs a unique `msgctxt`.
pub fn source_catalog(pot: impl Read) -> Result<Catalog, String> {
    let parsed = polib::po_file::parse_from_reader(pot).map_err(|e| e.to_string())?;
    let mut catalog = Catalog::new();
    for message in parsed.messages() {
        if message.is_plural() {
            return Err(format!(
                "{:?}: plural forms are not used (NOTES.md 6.9)",
                message.msgid()
            ));
        }
        let key = message
            .msgctxt()
            .filter(|key| !key.is_empty())
            .ok_or_else(|| format!("{:?}: missing msgctxt (the stable key)", message.msgid()))?;
        if catalog
            .insert(key.to_owned(), message.msgid().to_owned())
            .is_some()
        {
            return Err(format!("duplicate msgctxt {key:?}"));
        }
    }
    Ok(catalog)
}

/// Translated catalog from a `.po`: `msgctxt → msgstr`, only translated, non-fuzzy messages whose
/// key exists in `source` and whose `{placeholders}` match the source. The rest falls back to
/// English at runtime; problems are returned as warnings.
pub fn translated_catalog(
    po: impl Read,
    source: &Catalog,
) -> Result<(Catalog, Vec<String>), String> {
    let parsed = polib::po_file::parse_from_reader(po).map_err(|e| e.to_string())?;
    let mut catalog = Catalog::new();
    let mut warnings = Vec::new();
    for message in parsed.messages() {
        let Some(key) = message.msgctxt() else {
            continue;
        };
        if message.is_plural() || !message.is_translated() || message.is_fuzzy() {
            continue;
        }
        let Some(english) = source.get(key) else {
            warnings.push(format!("{key}: not in the source catalog (outdated .po?)"));
            continue;
        };
        let text = message.msgstr().map_err(|e| e.to_string())?;
        if placeholders(text) != placeholders(english) {
            warnings.push(format!(
                "{key}: placeholders differ from the English text, using English"
            ));
            continue;
        }
        catalog.insert(key.to_owned(), text.to_owned());
    }
    Ok((catalog, warnings))
}

/// `{name}` placeholders in a text (NOTES.md 6.9 conventions).
fn placeholders(text: &str) -> BTreeSet<&str> {
    let mut found = BTreeSet::new();
    let mut rest = text;
    while let Some(start) = rest.find('{') {
        let Some(len) = rest[start + 1..].find('}') else {
            break;
        };
        found.insert(&rest[start + 1..start + 1 + len]);
        rest = &rest[start + 1 + len + 1..];
    }
    found
}

#[cfg(test)]
mod tests {
    use super::*;

    const POT: &str = r#"
msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"

msgctxt "InstallButton"
msgid "Install"
msgstr ""

msgctxt "DownloadProgress"
msgid "{done} of {total} MB"
msgstr ""

msgctxt "Multi"
msgid ""
"Line one\n"
"Line \"two\""
msgstr ""
"#;

    #[test]
    fn source_catalog_uses_msgctxt_as_key() {
        let catalog = source_catalog(POT.as_bytes()).unwrap();
        assert_eq!(catalog.len(), 3);
        assert_eq!(catalog["InstallButton"], "Install");
        assert_eq!(catalog["Multi"], "Line one\nLine \"two\"");
    }

    #[test]
    fn source_catalog_rejects_missing_or_duplicate_keys() {
        assert!(source_catalog("msgid \"no key\"\nmsgstr \"\"\n".as_bytes()).is_err());
        let duplicate =
            "msgctxt \"A\"\nmsgid \"x\"\nmsgstr \"\"\n\nmsgctxt \"A\"\nmsgid \"y\"\nmsgstr \"\"\n";
        assert!(source_catalog(duplicate.as_bytes()).is_err());
    }

    #[test]
    fn translated_catalog_skips_untranslated_fuzzy_and_broken() {
        let source = source_catalog(POT.as_bytes()).unwrap();
        let po = r#"
msgid ""
msgstr ""
"Content-Type: text/plain; charset=UTF-8\n"

msgctxt "InstallButton"
msgid "Install"
msgstr "Installieren"

#, fuzzy
msgctxt "Multi"
msgid "Line one"
msgstr "Zeile"

msgctxt "DownloadProgress"
msgid "{done} of {total} MB"
msgstr "{done} von {gesamt} MB"

msgctxt "Gone"
msgid "Old"
msgstr "Alt"
"#;
        let (catalog, warnings) = translated_catalog(po.as_bytes(), &source).unwrap();
        assert_eq!(catalog.len(), 1);
        assert_eq!(catalog["InstallButton"], "Installieren");
        assert_eq!(warnings.len(), 2, "{warnings:?}"); // broken placeholder + unknown key
    }

    #[test]
    fn finds_placeholders() {
        assert_eq!(
            placeholders("{a} x {b}{c}"),
            BTreeSet::from(["a", "b", "c"])
        );
        assert!(placeholders("none { open").is_empty());
    }
}
