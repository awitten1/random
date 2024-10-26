## Random cgroup scripts

With docker
```
docker container run --privileged --cap-add=SYS_ADMIN --cgroupns=host -v $PWD:/learn -it ubuntu bash
```
Installs
```
apt update && apt install build-essential
```

Compile
```
g++ -O3 allocate.cpp -o allocate
g++ -O3 spinner.cpp -o spinner
```