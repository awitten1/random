import { argv } from 'node:process';
import { open } from 'node:fs/promises';
import { Buffer } from 'node:buffer';

const pid = argv[2]
const maps_fh = await open(`/proc/${pid}/maps`)
const pmaps_fh = await open(`/proc/${pid}/pagemap`)

const PAGE_SIZE = 4096

for await (const line of maps_fh.readLines()) {
  const range = line.split(' ')[0].split('-')
  const laddr = range[0]
  const raddr = range[1]
  console.log(laddr, raddr)

  const laddrn = parseInt(laddr, 16)
  const raddrn = parseInt(raddr, 16)
  const buf = Buffer.alloc(8);

  await pmaps_fh.read(buf, 0, 8, laddrn);
  const i = buf.readBigUInt64LE()
  console.log(i)

}