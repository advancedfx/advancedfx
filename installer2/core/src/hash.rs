use std::fs::File;
use std::io::{self, BufReader, Write};
use std::path::Path;

use sha2::{Digest, Sha256};

use crate::error::{IoContext, Result};

pub fn to_hex(bytes: &[u8]) -> String {
    bytes.iter().map(|b| format!("{b:02x}")).collect()
}

pub fn is_sha256_hex(s: &str) -> bool {
    s.len() == 64 && s.bytes().all(|b| matches!(b, b'0'..=b'9' | b'a'..=b'f'))
}

/// Returns SHA-256 (lower-case hex) and size of a file.
pub fn sha256_file(path: &Path) -> Result<(String, u64)> {
    let file = File::open(path).ctx("open", path)?;
    let mut hasher = Sha256::new();
    let size = io::copy(&mut BufReader::new(file), &mut hasher).ctx("read", path)?;
    Ok((to_hex(&hasher.finalize()), size))
}

/// Writer that hashes and counts everything written through it.
pub struct HashingWriter<W: Write> {
    inner: W,
    hasher: Sha256,
    len: u64,
}

impl<W: Write> HashingWriter<W> {
    pub fn new(inner: W) -> Self {
        Self {
            inner,
            hasher: Sha256::new(),
            len: 0,
        }
    }

    /// Flushes and returns SHA-256 (lower-case hex), length and the inner writer.
    pub fn finish(self) -> io::Result<(String, u64, W)> {
        let Self {
            mut inner,
            hasher,
            len,
        } = self;
        inner.flush()?;
        Ok((to_hex(&hasher.finalize()), len, inner))
    }
}

impl<W: Write> Write for HashingWriter<W> {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        let written = self.inner.write(buf)?;
        self.hasher.update(&buf[..written]);
        self.len += written as u64;
        Ok(written)
    }

    fn flush(&mut self) -> io::Result<()> {
        self.inner.flush()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn hashing_writer_matches_known_hash() {
        let mut writer = HashingWriter::new(Vec::new());
        writer.write_all(b"abc").unwrap();
        let (hash, len, inner) = writer.finish().unwrap();
        assert_eq!(
            hash,
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        );
        assert_eq!(len, 3);
        assert_eq!(inner, b"abc");
        assert!(is_sha256_hex(&hash));
        assert!(!is_sha256_hex("ABC"));
    }
}
