# CS 395T Final Project

This is the code for a modified Istio Envoy proxy that implements compression at the TCP level. We're using it to test redundancy elimination in microservices.

## About the code

There are two new filters in this implementation, stored in `source/extensions/filters/network`.
* The `jb_test` filter is used for logging information about connection data. It's named that way because I was using it to figure out how to add C++ filters to Envoy.
* The `tcp_compression` filter applies GZIP compression to a TCP stream. It's implemented on top of Envoy's compression API. It's currently set to use GZIP, but the code is generic enough that you could easily instantiate another Envoy compressor and decompressor instead (Envoy has zlib and Brotli support built in).

The `jbtestfilter.yaml` file includes an example of how to configure the TCP compression filter. The `jbtestfilter-logonly.yaml` file includes an example of the jb_test logging filter.

## Building and testing

I hacked together a lot of the test environment, so the scripts may need some tweaking to run on your system.

If you're just running the system without modifying the C++ code, you can use my pre-built images. Install Istio using the normal process, except add `--set hub=jbyoshi --set tag=latest` to the `istioctl install` command.

If you're building the C++ code yourself:
* You may need a Docker Hub account to build and test, unless you're doing everything on a single machine. Change the HUB variable everywhere it appears from "jbyoshi" to whatever your Docker Hub username is. If you don't want to use Docker Hub, remove all the references to "docker login" and "push" in the scripts; however, I still recommend picking something unique as the HUB variable.
* To build and deploy normally, you should have this repository and https://github.com/istio/istio both cloned into the same parent directory. The Istio checkout should be named "istio". (setup-build.sh is intended to help with this setup.)
* To (semi-)quickly compile or recompile changes, run build.sh. Note that the first build takes several hours and will gobble many gigabytes of cache that you'll need to keep around.

Once you have Istio set up, you can enable the filter using `kubectl apply -f <filter file>.yaml` AND `kubectl apply -f <filter file>.yaml -n istio-system`. Make sure to run both, since Kubernetes will only apply the configuration to one namespace per `apply` command, and Istio's ingress and egress gateways are in the separate `istio-system` namespace.
