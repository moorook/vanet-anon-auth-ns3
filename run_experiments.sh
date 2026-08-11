#!/usr/bin/env bash
set -euo pipefail
NS3_DIR="${NS3_DIR:-/opt/ns-3.46.1}"
cd "$NS3_DIR"
mkdir -p results
./ns3 build
for n in 50 100 150; do
  ./ns3 run "scratch/vanet-v2x-anon-auth --numVehicles=${n} --numRSU=5"
done
