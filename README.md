# V2X Anonymous Authentication Simulation

Reproducibility package for the ns-3 evaluation of the proposed anonymous-credential V2X/VANET authentication framework.

## Environment
- ns-3.46.1
- IEEE 802.11p
- 2-km highway
- ConstantVelocityMobilityModel
- Standard sweep: 50/100/150 vehicles, 5 RSUs
- Dense sweep: 200/300/400 vehicles and 5/10/20 RSUs

## Important scope statement
The simulator does **not** implement the cryptographic primitives themselves. Credential recovery, credential randomization, ElGamal identity encapsulation, and NIZK generation/verification are abstracted as configurable authentication-processing delays. The experiment therefore evaluates event-level authentication latency and scalability under the adopted network configuration. **No SUMO results are claimed.**

## Run
Copy `scratch/vanet-v2x-anon-auth.cc` into the ns-3.46.1 `scratch/` directory, then run:

```bash
./ns3 build
bash scripts/run_experiments.sh
```

For ultra-dense scenarios with varying RSU density and higher message-generation rates:

```bash
bash scripts/run_dense_scenarios.sh
```

Analyze CSV output:

```bash
python3 scripts/analyze_results.py
```

Install Python dependency if needed:

```bash
pip install matplotlib
```

## GitHub
The repository is intended to be released as supplementary reproducibility material. Do not upload generated CSV/PNG files unless you want to archive a specific run; `.gitignore` excludes them by default.
