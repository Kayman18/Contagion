#!/usr/bin/env python3
"""Check target kernel placement and executable-RAM budget from linked symbols."""
import json,subprocess,sys
out=subprocess.check_output([sys.argv[1],'-t',sys.argv[2]],text=True)
required=['vc_render_stereo','vc_mix_sources','vc_sub_next','vc_osc_next','vc_filter_setup','vc_filter_next','vc_env_next','vc_route_sample','vc_profile_run','vc_noise_next','vc_fm_control','vc_fm_increment','vc_pitch_env_control','vc_lfo_tick','vc_lfo_shape','vc_lfo2_osc_control','vc_lfo2_pan_control','vc_osc3_control','vc_sat_next', 'vc_filter_control','vc_distortion_next','vc_eq_next','vc_boost_next','vc_pitch_bases','vc_pwm_control']
rows={line.split()[-1]:line.split() for line in out.splitlines() if line.split()}
for name in required:
 assert '.voice_hot' in rows[name],name
start=int(rows['_voice_hot_start'][0],16);end=int(rows['_voice_hot_end'][0],16)
assert 0x1c00000<=start<end<=0x1c06000
print(json.dumps({'kernels_in_executable_ram':required,'kernel_bytes':end-start,'total_executable_ram_used':end-0x1c00000,'executable_ram_capacity':0x6000,'pass':True},indent=2))
