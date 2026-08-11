#!/usr/bin/env bash
set -euo pipefail
NS3_DIR="${NS3_DIR:-/opt/ns-3.46.1}"
cd "$NS3_DIR"
mkdir -p results
./ns3 build
for n in 200 300 400; do
  for r in 5 10 20; do
    ./ns3 run "scratch/vanet-v2x-anon-auth --numVehicles=${n} --numRSU=${r} --simulationTime=30 --v2iInterval=0.02 --v2vInterval=0.02 --v2vEvents=200"
  done
done
