#!/bin/sh
# This assumes Kubernetes was already set up.

cd ~
curl -L https://istio.io/downloadIstio | sh -
cd istio-*
bin/istioctl install --set profile=demo --set hub=jbyoshi --set tag=latest -y
kubectl label namespace default istio-injection=enabled
kubectl apply -f samples/bookinfo/platform/kube/bookinfo.yaml
kubectl apply -f samples/bookinfo/networking/bookinfo-gateway.yaml

export INGRESS_HOST=$(kubectl -n istio-system get service istio-ingressgateway -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
export INGRESS_PORT=$(kubectl -n istio-system get service istio-ingressgateway -o jsonpath='{.spec.ports[?(@.name=="http2")].port}')
export SECURE_INGRESS_PORT=$(kubectl -n istio-system get service istio-ingressgateway -o jsonpath='{.spec.ports[?(@.name=="https")].port}')
export GATEWAY_URL=$INGRESS_HOST:$INGRESS_PORT

kubectl apply -f samples/addons
kubectl rollout status deployment/kiali -n istio-system

echo "http://$GATEWAY_URL/productpage"