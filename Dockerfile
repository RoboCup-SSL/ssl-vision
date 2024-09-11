FROM ubuntu:22.04

# Set environment variables to avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set Qt to use the offscreen platform
ENV QT_QPA_PLATFORM=offscreen

# Pre-set timezone to Europe/Stockholm to avoid tzdata interactive prompt
RUN ln -fs /usr/share/zoneinfo/Europe/Stockholm /etc/localtime && \
    echo "Europe/Stockholm" > /etc/timezone && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata

# Install required system packages
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    git \
    screen \
    sudo

RUN pip install opencv-python requests

# Clone the GitHub repository
RUN git clone https://github.com/LiU-SeeGoals/ssl-vision.git

# Change working directory to the cloned repository
WORKDIR /ssl-vision

# Switch to the correct branch
RUN git switch image-file-reading

RUN ./InstallPackagesUbuntu.sh

RUN make configure_vapix

RUN make

#&& tail -f /dev/null
CMD ["screen", "-c", "screen.camera"]