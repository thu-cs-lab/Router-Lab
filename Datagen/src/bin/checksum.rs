extern crate datagen;
extern crate pcap_file;
extern crate rand;
extern crate structopt;

use pcap_file::*;
use rand::Rng;
use std::fs::File;
use std::io::BufWriter;
use std::io::Write;
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
struct Opt {
    /// file names are {input,output}${index}.pcap
    #[structopt(name = "index")]
    index: String,

    /// number of packets for each type
    #[structopt(name = "packets")]
    packets: usize,

    /// allow generation of bad IP packets
    #[structopt(long = "ip")]
    ip: bool,

    /// append more bytes
    #[structopt(long = "large")]
    large: bool,
}

fn main() {
    let opt = Opt::from_args();
    println!("options: {:?}", opt);
    let mut rng = rand::thread_rng();
    let file = File::create(format!("checksum_input{}.pcap", opt.index)).unwrap();
    let ans_file = File::create(format!("checksum_output{}.out", opt.index)).unwrap();
    let mut header = PcapHeader::with_datalink(DataLink::ETHERNET);
    header.snaplen = 0x40000;
    let mut writer = PcapWriter::with_header(header, file).unwrap();
    let mut ans_writer = BufWriter::new(ans_file);

    for i in 0..opt.packets {
        let sec = i as u32;
        let types = if opt.ip { 2 } else { 1 };
        let file_size = if opt.large { 1000 } else { 10 };
        let packet_type = rng.gen_range(0, types);
        let frame = match packet_type {
            0 => {
                // good IP
                let mut data = datagen::parse_string("01005e000001c0562718cc8e81000001080046c00020000040000102041700000000e0000001940400001164ee9b00000000");
                let extra_length = rng.gen_range(0, file_size);
                for _ in 0..extra_length {
                    data.push(rng.gen());
                }
                data = datagen::update_ip_checksum(data);
                write!(ans_writer, "Yes\n").expect("write");
                data
            }
            1 => {
                // bad IP
                let mut data = datagen::parse_string("74EAC8233333 001217122345 8100 0001 080045000020000040004011DD63B7AD71B701020304E36E2766000C635A3132330A");
                let extra_length = rng.gen_range(0, file_size);
                for _ in 0..extra_length {
                    data.push(rng.gen());
                }
                data = datagen::update_ip_checksum(data);
                data[28] ^= rng.gen_range(1, 255);
                data[29] ^= rng.gen_range(1, 255);
                write!(ans_writer, "No\n").expect("write");
                data
            }
            _ => unimplemented!(),
        };
        writer
            .write_packet(&Packet::new_owned(sec, 0, frame.len() as u32, frame))
            .unwrap();
    }
}
