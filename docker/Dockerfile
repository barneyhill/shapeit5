FROM ubuntu:20.04

LABEL org.opencontainers.image.created="2022-03-16"
LABEL org.opencontainers.image.url="https://github.com/odelaneau/shapeit5"
LABEL org.opencontainers.image.version="1.0.0"
LABEL org.opencontainers.image.licences="MIT"
LABEL org.opencontainers.image.title="shapeit5"
LABEL org.opencontainers.image.authors="olivier.delaneau@unil.ch"

# set the environment variables
ENV HOME /root

ENV htslib_version 1.15
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

ARG DEBIAN_FRONTEND=noninteractive

# run update and install necessary tools from package manager
WORKDIR /tmp
RUN apt-get update && apt-get -y upgrade && apt-get install -y \
	apt-utils time curl gnupg gcc g++ make autoconf && \
	apt-get clean && apt-get purge && \
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN apt-get update && apt-get -y upgrade && apt-get install -y \
	git zlib1g-dev bzip2 libbz2-dev liblzma-dev && \
	apt-get clean && apt-get purge && \
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# bcftools
RUN curl -SL https://github.com/samtools/bcftools/releases/download/${htslib_version}/bcftools-${htslib_version}.tar.bz2 \
    > bcftools-${htslib_version}.tar.bz2 && \
    tar -xjvf bcftools-${htslib_version}.tar.bz2 && \
    cd /tmp/bcftools-${htslib_version} && \
    ./configure && \
    make && \
    make install && \
    rm -rf bcftools-${htslib_version}.tar.bz2 && \
    rm -rf bcftools-${htslib_version}

# get GLIMPSE v2.0.0 [binary]
ADD resources/SHAPEIT5_phase_common_static /bin/SHAPEIT5_phase_common_static
ADD resources/SHAPEIT5_phase_rare_static /bin/SHAPEIT5_phase_rare_static
ADD resources/SHAPEIT5_switch_static /bin/SHAPEIT5_switch_static
ADD resources/SHAPEIT5_ligate_static /bin/SHAPEIT5_ligate_static

RUN chmod +x /bin/SHAPEIT5*

WORKDIR $HOME
