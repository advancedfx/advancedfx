use std::cmp::Ordering;

use crate::error::{Error, Result};

/// Parses a dotted numeric version like `2.192.8`.
pub fn parse(version: &str) -> Result<Vec<u64>> {
    version
        .split('.')
        .map(|part| part.parse::<u64>().map_err(|_| Error::InvalidVersion(version.to_owned())))
        .collect()
}

/// Compares dotted numeric versions, missing parts count as 0 (`2.192` == `2.192.0`).
pub fn compare(a: &str, b: &str) -> Result<Ordering> {
    let (a, b) = (parse(a)?, parse(b)?);
    for i in 0..a.len().max(b.len()) {
        let (x, y) = (a.get(i).copied().unwrap_or(0), b.get(i).copied().unwrap_or(0));
        if x != y {
            return Ok(x.cmp(&y));
        }
    }
    Ok(Ordering::Equal)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn compares_numerically() {
        assert_eq!(compare("2.192.8", "2.192.10").unwrap(), Ordering::Less);
        assert_eq!(compare("2.193.0", "2.192.99").unwrap(), Ordering::Greater);
        assert_eq!(compare("2.192", "2.192.0").unwrap(), Ordering::Equal);
        assert!(compare("2.x", "2.0").is_err());
        assert!(compare("", "2.0").is_err());
    }
}
