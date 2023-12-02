#!/bin/sh -e
# Recompiles Envoy, builds it into Docker containers, and pushes it to Docker Hub.
# You should have github.com/istio/istio cloned.

# Change this to your username on Docker Hub. You need to be logged in with "docker login".
HUB=jbyoshi

sudo BUILD_WITH_CONTAINER=1 make build exportcache
cd ../istio
mkdir -p out/linux_amd64
cp ../istio-proxy/out/linux_amd64/envoy out/linux_amd64/envoy
cp ../istio-proxy/out/linux_amd64/envoy out/linux_amd64/release/envoy
sudo BUILD_WITH_CONTAINER=1 HUB=$HUB TAG=latest make build docker.push

# Then run this on a target machine to install:
# istioctl install --set hub=$HUB --set tag=latest

# For disk space management
echo "Build succeeded. Do you also want to do 'docker system prune'?"
docker system prune  # includes a prompt
