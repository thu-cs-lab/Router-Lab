#!/bin/sh
IMAGE_NAME=jiegec/router-lab-$(uname -m)
sudo docker build -t $IMAGE_NAME .
sudo docker push $IMAGE_NAME

