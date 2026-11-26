// Resource groundwork only; no EQ/distortion DSP is linked by this script.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),cp=require('child_process'),assert=require('assert');
const root=path.resolve(__dirname,'..'),raw=fs.readFileSync(root+'/reference/virus_fx.wasm');
assert.equal(crypto.createHash('sha256').update(raw).digest('hex'),'2fd986aab9caad2eee8186d11cfd272eb99180e4cb70c51a4ad47d49dc5d0e17');
let x;const wasi={environ_sizes_get(a,b){const d=new DataView(x.memory.buffer);d.setUint32(a,0,true);d.setUint32(b,0,true);return 0},environ_get:()=>0,fd_close:()=>0,fd_seek:()=>0,fd_write:()=>0,proc_exit:c=>{throw Error(c)}};
x=new WebAssembly.Instance(new WebAssembly.Module(raw),{wasi_snapshot_preview1:wasi}).exports;x._initialize();
// func77 uses base 16976208 with pair index frequency + Q*128. Each original
// coefficient is an unsigned 24-bit word, sign-extended at use, scaled by 2^-23.
const table=Buffer.from(x.memory.buffer,16976208,128*128*2*4),packed=Buffer.alloc(128*128*2*3);
for(let i=0;i<32768;i++){const v=table.readUInt32LE(i*4);assert(v<=0xffffff);packed.writeUIntLE(v,i*3,3);assert.equal(packed.readUIntLE(i*3,3),v);assert.equal((packed.readUIntLE(i*3,3)<<8)>>8,(v<<8)>>8);}
fs.writeFileSync(__dirname+'/eq-mid-q23.bin',packed);
let symbols=cp.execFileSync(process.argv[2],['-t',root+'/app/build/virooos.elf'],{encoding:'utf8'});
const row=symbols.split('\n').find(l=>l.trim().endsWith(' vc_lfo_waves'));assert(row);const lfoBytes=parseInt(row.trim().split(/\s+/).at(-2),16);assert.equal(lfoBytes,40*2049*4);
const current=JSON.parse(fs.readFileSync(root+'/validation/test44-memory-validation.json'));
const activeLfo=3*2049*4,fxStateReservation=1024;
console.log(JSON.stringify({baseline:'TEST44',retained:['EQ','distortion'],excluded:['reverb','delay','chorus','phaser'],original_eq_block_including_boost_bytes:408,original_distortion_block_all_modes_bytes:80,proposed_native_fx_state_reservation:fxStateReservation,mid_eq:{original_bytes:table.length,packed_bytes:packed.length,words_verified:32768,lossless:true,sha256:crypto.createHash('sha256').update(packed).digest('hex')},current_free_ram:current.total_free_data,current_application_flash_free:3788,proposal:{implemented:false,embedded_lfo_bytes_removable:lfoBytes,three_active_lfo_tables_ram:activeLfo,remaining_ram_after_tables_and_fx_reservation:current.total_free_data-activeLfo-fxStateReservation,application_flash_after_removal:3788+lfoBytes,application_flash_after_removal_and_packed_mid_eq:3788+lfoBytes-packed.length,not_yet_counted:['LFO loading control/state and code','remaining EQ/distortion tables and native DSP code','UI/MIDI integration','alignment'],tradeoff:'Selecting a different LFO wavetable would require muted preparation; playback would still read decoded float32 samples.'},pass:true},null,2));
