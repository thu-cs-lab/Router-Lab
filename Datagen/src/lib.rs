pub fn parse_string(input: &str) -> Vec<u8> {
    let mut cur = 0;
    let mut index = 0;
    let mut result = Vec::new();
    for ch in input.bytes() {
        let num = if ch >= b'0' && ch <= b'9' {
            ch - b'0'
        } else if ch >= b'a' && ch <= b'f' {
            ch - b'a' + 10
        } else if ch >= b'A' && ch <= b'F' {
            ch - b'A' + 10
        } else {
            continue;
        };

        if index == 0 {
            index = 1;
            cur = num << 4;
        } else {
            result.push(cur | num);
            index = 0;
        }
    }
    result
}

pub fn update_ip_checksum(mut data: Vec<u8>) -> Vec<u8> {
    // calculate ip checksum
    assert_eq!(data[16], 0x08);
    assert_eq!(data[17], 0x00);
    //assert_eq!(data[18], 0x45);
    data[28] = 0;
    data[29] = 0;
    let mut checksum = 0;
    let mut len = ((data[18] & 0xF) * 2) as usize;
    for i in 0..len {
        checksum += ((data[18 + i * 2] as u32) << 8) | (data[18 + i * 2 + 1] as u32);
    }
    while checksum >= 0x10000 {
        checksum -= 0x10000;
        checksum += 1;
    }
    data[28] = !((checksum >> 8) as u8);
    data[29] = !(checksum as u8);

    data
}
