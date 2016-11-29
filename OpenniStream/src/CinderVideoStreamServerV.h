/*
 CinderVideoStreamServer.h
 
 Created by Vladimir Gusev on 5/19/12.
 Copyright (c) 2012 onewaytheater.us
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#ifndef CinderVideoStreamServerV_CinderVideoStreamServerV_h
#define CinderVideoStreamServerV_CinderVideoStreamServerV_h
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "PixelEntry.h"


using namespace boost::asio;
template <class T>
class CinderVideoStreamServerV{
public:
    
    CinderVideoStreamServerV(unsigned short port, ph::ConcurrentQueue<PixelEntry<T> >* queueToServer, size_t maxSize)
    :mSocket(mIOService),mAcceptor(mIOService,ip::tcp::endpoint(ip::tcp::v4(), port)),
    mQueue(queueToServer), maxSize(maxSize){
        boost::asio::socket_base::reuse_address option(true);
        mAcceptor.set_option(option);
        data = new T[maxSize];
    }
    ~CinderVideoStreamServerV(){
        if (data) delete [] data;
    }
    void run(){
        
        PixelEntry<T> entry;

        boost::system::error_code ignored_error;
        
        while(true){
            if (mQueue->try_pop(entry)){
                std::vector<T> vec = getDensePixels(entry.pixels, entry.size);
                memcpy( data, &vec[0], sizeof( T ) * vec.size() );
                const mutable_buffer image_buffer(data, vec.size()*sizeof(T));
                mAcceptor.accept(mSocket);
                boost::asio::write(mSocket, buffer(image_buffer), transfer_all(), ignored_error);
                mSocket.close();
            }
        }
    }
private:
    io_service mIOService;
    ip::tcp::socket mSocket;
    ip::tcp::acceptor mAcceptor;
    ph::ConcurrentQueue<PixelEntry<T> >* mQueue;
    std::size_t maxSize;
    T* data;
    
    std::vector<T> getDensePixels(T* pixels, size_t size){
        std::vector<T> densePixels;
        for (int i = 3; i < size; i+=1){
            if (pixels[i-3] == 0 && pixels[i-2]==0 && pixels[i-1]==0 && pixels[i]>0){
                densePixels.push_back(UINT16_MAX);
                size_t first = i / 1000;
                size_t second = i - (1000*first);
                
                densePixels.push_back(first);
                densePixels.push_back(second);
                densePixels.push_back(pixels[i]);
            } else if (pixels[i]>0){
                densePixels.push_back(pixels[i]);
            }
        }
        return densePixels;
    }
    
};

#endif
