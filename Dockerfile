FROM alpine
ADD ./src /src
ADD ./include /include
ADD ./Makefile ./
WORKDIR ./
CMD ["make"]
