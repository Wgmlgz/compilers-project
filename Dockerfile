FROM emscripten/emsdk


RUN apt-get update && \
    apt-get install -y \
    g++ \
    make \
    flex \
    bison \
    && rm -rf /var/lib/apt/lists/*

# RUN git clone https://github.com/emscripten-core/emsdk.git
# # Download and install the latest SDK tools.
# RUN cd ./emsdk
# RUN chmod +x ./emsdk
# RUN ./emsdk install latest
# RUN chmod +x ./emsdk_env.sh
# RUN source ./emsdk_env.sh
WORKDIR /workspace