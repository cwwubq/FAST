#include "ClariusStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <listen/listen.h>
#include <functional>

namespace fast {


ClariusStreamer::ClariusStreamer() {
    createOutputPort<Image>(0);
    mNrOfFrames = 0;
    mHasReachedEnd = false;
    mFirstFrameIsInserted = false;
    mStreamIsStarted = false;
    mIsModified = true;
}

void ClariusStreamer::execute() {
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        mStop = false;
        mThread = std::make_unique<std::thread>(std::bind(&ClariusStreamer::producerStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void ClariusStreamer::newImageFn(const void* newImage, const ClariusImageInfo* nfo, int npos, const ClariusPosInfo* pos) {

}

void testNewImageFn(ClariusStreamer::pointer streamer, const void* newImage, const ClariusImageInfo* nfo, int npos, const ClariusPosInfo* pos) {
    std::cout << "new image (" << newImage << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel
          << "bits. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos << std::endl;

    streamer->newImageFn(newImage, nfo, npos, pos);
}


void ClariusStreamer::producerStream() {
    reportInfo() << "Trying to set up Clarius streaming..." << reportEnd();

    int argc = 1;
    char** argv;
    std::string keydir = "/tmp/";
    static ClariusStreamer::pointer self = std::dynamic_pointer_cast<ClariusStreamer>(mPtr.lock());
    int success = clariusInitListener(argc, argv, keydir.c_str(),
            // new image callback
            [](const void* img, const ClariusImageInfo* nfo, int npos, const ClariusPosInfo*)
          {
            std::cout << "new image (" << img << "): " << nfo->width << " x " << nfo->height << " @ " << nfo->bitsPerPixel
            << "bits. @ " << nfo->micronsPerPixel << " microns per pixel. imu points: " << npos << std::endl;

            auto image = Image::New();
            image->create(nfo->width, nfo->height, TYPE_UINT8, 1, img);

            self->addOutputData(0, image);
          },
          nullptr, nullptr, nullptr, nullptr);
    if(success < 0)
        throw Exception("Unable to initialize clarius listener");

    std::string ipAddr = "192.168.1.1";
    uint port = 14111;
    success = clariusConnect(ipAddr.c_str(), port, nullptr);
    if(success < 0)
        throw Exception("Unable to connect to clarius scanner");

    while(true) {
        {
            // Check if stop signal is sent
            std::unique_lock<std::mutex> lock(mStopMutex);
            if(mStop) {
                mStreamIsStarted = false;
                mFirstFrameIsInserted = false;
                mHasReachedEnd = false;
                break;
            }
        }

        // Get image from newImageFn and add to output
        std::this_thread::sleep_for(std::chrono::duration<double>(1));
        std::cout << "waiting.." << std::endl;
    }

    success = clariusDisconnect(nullptr);
    if(success < 0)
        throw Exception("Unable to disconnect from clarius scanner");

    success = clariusDestroyListener();
    if(success < 0)
        throw Exception("Unable to destroy clarius listener");

    reportInfo() << "Clarius streamer stopped" << Reporter::end();
}

bool ClariusStreamer::hasReachedEnd() {
    return mHasReachedEnd;
}

uint ClariusStreamer::getNrOfFrames() {
    return mNrOfFrames;
}

ClariusStreamer::~ClariusStreamer() {
    if(mStreamIsStarted) {
        stop();
        mThread->join();
    }
}

void ClariusStreamer::stop() {
    std::unique_lock<std::mutex> lock(mStopMutex);
    reportInfo() << "Stopping real sense streamer" << Reporter::end();
    mStop = true;
}

}
