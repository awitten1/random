## Random cgroup scripts

With docker
```
docker container run --privileged --cap-add=SYS_ADMIN --cgroupns=host -v $PWD:/learn -it ubuntu bash
apt update && apt install build-essentials
g++ -O3 allocate.cpp -o allocate
```