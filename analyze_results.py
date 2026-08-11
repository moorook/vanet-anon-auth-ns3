#!/usr/bin/env python3
import csv, glob, os
import matplotlib.pyplot as plt
RESULT_DIR=os.path.join(os.path.dirname(__file__),'..','results')
def read_result(fn):
    d={}
    with open(fn,encoding='utf-8') as f:
        for row in list(csv.reader(f))[1:]:
            if len(row)>=2:
                try:d[row[0]]=float(row[1])
                except ValueError:d[row[0]]=row[1]
    return d
def main():
    fs=glob.glob(os.path.join(RESULT_DIR,'auth_*.csv'))
    if not fs: print('No result files found.'); return
    rs=sorted([read_result(f) for f in fs],key=lambda x:(int(x['Vehicles']),int(x['RSUs'])))
    print('Vehicles | RSUs | V2I Success | V2I Delay | V2V Success | V2V Delay')
    for r in rs: print(f"{int(r['Vehicles']):8d} | {int(r['RSUs']):4d} | {r['V2I_SuccessRate']:11.2f}% | {r['V2I_AverageDelay_ms']:9.4f} | {r['V2V_SuccessRate']:11.2f}% | {r['V2V_AverageDelay_ms']:8.4f}")
    mainrs=[r for r in rs if int(r['RSUs'])==5] or rs
    x=[int(r['Vehicles']) for r in mainrs]
    for name,a,b,y in [('authentication_success_rate.png',[r['V2I_SuccessRate'] for r in mainrs],[r['V2V_SuccessRate'] for r in mainrs],'Authentication Success Rate (%)'),('authentication_delay.png',[r['V2I_AverageDelay_ms'] for r in mainrs],[r['V2V_AverageDelay_ms'] for r in mainrs],'Average Authentication Delay (ms)')]:
        plt.figure(); plt.plot(x,a,marker='o',label='V2I'); plt.plot(x,b,marker='s',label='V2V'); plt.xlabel('Number of Vehicles'); plt.ylabel(y); plt.legend(); plt.grid(True,alpha=.3); plt.tight_layout(); plt.savefig(os.path.join(RESULT_DIR,name),dpi=300); plt.close()
    y=[r['V2I_Completed']+r['V2V_Completed'] for r in mainrs]
    plt.figure(); plt.plot(x,y,marker='o'); plt.xlabel('Number of Vehicles'); plt.ylabel('Completed Authentication Events'); plt.grid(True,alpha=.3); plt.tight_layout(); plt.savefig(os.path.join(RESULT_DIR,'authentication_scalability.png'),dpi=300); plt.close()
if __name__=='__main__': main()
