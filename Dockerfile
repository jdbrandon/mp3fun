FROM alpine
ADD ./src /src
ADD ./include /include
ADD ./Makefile ./
WORKDIR ./
CMD ["make"]
CMD ["cd", "./test"]
CMD ["make"]
CMD ["./mp3fun-test-driver"]
