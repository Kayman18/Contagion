/* Actual saved TEST05 step retained for exact comparisons. */
float vc_osc_next_test05(VCOsc *o,float inc,float sync){
 float p=o->phase,x=vc_osc_raw(o,p)+o->pending;o->pending=0;
 if(inc>0){float end=sync>=0?sync:1;
  if(o->saw_gain>0){float t=(1-p)/inc;if(t<=end)x+=vc_osc_step(o,t,-2*o->saw_gain);}
  if(o->pulse_gain>0){float q=p+o->duty;if(q>=1)q-=1;float t=(1-q)/inc;if(t<=end)x+=vc_osc_step(o,t,2*o->pulse_gain);}}
 if(sync>=0){float q=p+inc*sync;q-=floorf(q);x+=vc_osc_step(o,sync,vc_osc_raw(o,0)-vc_osc_raw(o,q));o->phase=inc*(1-sync);}else o->phase=p+inc;
 if(o->phase>=1||o->phase<0)o->phase-=floorf(o->phase);
 return x;
}
