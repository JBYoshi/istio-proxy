#!/bin/sh
set -e
cd $HOME

# HUB needs to be set to an account on hub.docker.com.
export HUB=jbyoshi

sudo apt update
sudo apt install -y docker.io
docker login
# NOTE: this doesn't take effect until after relogging, so the Docker commands below still need sudo.
sudo usermod --groups docker --append $(whoami)

git clone https://github.com/istio/istio istio
#git clone https://github.com/istio/proxy istio-proxy  # That's this repo
cd istio-proxy
sudo BUILD_WITH_CONTAINER=1 make build exportcache
cd ../istio
mkdir -p out/linux_amd64/release
cp ../istio-proxy/out/linux_amd64/envoy out/linux_amd64/envoy
cp ../istio-proxy/out/linux_amd64/envoy out/linux_amd64/release/envoy

sudo BUILD_WITH_CONTAINER=1 HUB=$HUB make build docker.push
