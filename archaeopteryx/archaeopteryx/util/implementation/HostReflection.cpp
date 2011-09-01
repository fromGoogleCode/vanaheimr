/*	\file   HostReflection.cpp
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date   Saturday July 16, 2011
	\brief  The source file for the HostReflection set of functions.
*/

#pragma once

// Archaeopteryx Includes
#include <archaeopteryx/util/interface/HostReflection.h>
#include <archaeopteryx/util/interface/ThreadId.h>
#include <archaeopteryx/util/interface/StlFunctions.h>
#include <archaeopteryx/util/interface/debug.h>

// Standard Library Includes
#include <cstring>
#include <cassert>

// Preprocessor Macros
#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 1

namespace util
{

// TODO Remove these when __device__ can be embedded in a clas
__device__ HostReflection::DeviceQueue* _hostToDevice;
__device__ HostReflection::DeviceQueue* _deviceToHost;

// device/host shared memory region
static char* _deviceHostSharedMemory = 0;

__device__ void HostReflection::sendSynchronous(const Message& m)
{
	unsigned int bytes = m.payloadSize();

	char* buffer = new char[bytes + sizeof(SynchronousHeader)];
	
	SynchronousHeader* header = reinterpret_cast<SynchronousHeader*>(buffer);
	
	header->type     = Synchronous;
	header->threadId = threadId();
	header->size     = bytes;
	header->handler  = m.handler();
	
	volatile bool* flag = new bool;
	*flag = false;

	header->address = (void*)flag;	

	std::memcpy(buffer + sizeof(SynchronousHeader), m.payload(),
		m.payloadSize());
	 
	printf(" sending synchronous gpu->host message "
		"(%d type, %d id, %d size, %d handler)\n", Synchronous, threadId(),
		bytes, m.handler());
	
	while(!_deviceToHost->push(buffer, bytes));

	printf("  waiting for ack...\n");
	
	while(*flag == false);

	printf("   ...received ack\n");
	
	delete flag;
	delete[] buffer;
}

__device__ void HostReflection::receive(Message& m)
{
	while(!_hostToDevice->peek());

	size_t bytes = m.payloadSize() + sizeof(Header);

	char* buffer = new char[bytes];
	
	_hostToDevice->pull(buffer, bytes);

	std::memcpy(m.payload(), buffer, m.payloadSize());

	delete[] buffer;
}

__device__ size_t HostReflection::maxMessageSize()
{
	return 64;
}

__host__ void HostReflection::create()
{
	assert(_booter == 0);
	_booter = new BootUp;
}

__host__ void HostReflection::destroy()
{
	delete _booter;
}

__host__ void HostReflection::handleOpenFile(const Message*)
{

}

__host__ void HostReflection::handleTeardownFile(const Message*)
{

}

__host__ void HostReflection::handleFileWrite(const Message*)
{

}

__host__ void HostReflection::handleFileRead(const Message*)
{

}

__host__ HostReflection::HostQueue::HostQueue(QueueMetaData* m)
: _metadata(m)
{

}

__host__ HostReflection::HostQueue::~HostQueue()
{

}

__host__ bool HostReflection::HostQueue::push(const void* data, size_t size)
{
	if(size > _capacity()) return false;

	size_t end  = _metadata->size;
	size_t head = _metadata->head;

	size_t remainder = end - head;
	size_t firstCopy = min(remainder, size);

	std::memcpy(_metadata->hostBegin + head, data, firstCopy);

	bool secondCopyNecessary = firstCopy != size;

	size_t secondCopy = secondCopyNecessary ? size - firstCopy : 0;
	
	std::memcpy(_metadata->hostBegin, (char*)data + firstCopy, secondCopy);
	_metadata->head = secondCopyNecessary ? secondCopy : head + firstCopy;
	
	return true;
}

__host__ bool HostReflection::HostQueue::pull(void* data, size_t size)
{
	report("   pulling " << size << " bytes from host queue (" << _used()
		<< " used, " << _capacity() << " remaining, " << this->size()
		<< " size)");

	if(size > _used()) return false;

	_metadata->tail = _read(data, size);

	report("    after pull (" << _used()
		<< " used, " << _capacity() << " remaining, " << this->size()
		<< " size)");

	return true;
}

__host__ bool HostReflection::HostQueue::peek()
{
	return _used() >= sizeof(Header);
}

__host__ size_t HostReflection::HostQueue::size() const
{
	return _metadata->size;
}

__host__ size_t HostReflection::HostQueue::_used() const
{
	size_t end  = _metadata->size;
	size_t head = _metadata->head;
	size_t tail = _metadata->tail;
	
	size_t greaterOrEqual = head - tail;
	size_t less           = (tail) + (end - head);
	
	bool isGreaterOrEqual = head >= tail;
	
	return (isGreaterOrEqual) ? greaterOrEqual : less;
}

__host__ size_t HostReflection::HostQueue::_capacity() const
{
	return size() - _used();
}

__host__ size_t HostReflection::HostQueue::_read(void* data, size_t size)
{
	size_t end  = _metadata->size;
	size_t tail = _metadata->tail;

	size_t remainder = end - tail;
	size_t firstCopy = min(remainder, size);

	std::memcpy(data, _metadata->hostBegin + tail, firstCopy);

	bool secondCopyNecessary = firstCopy != size;

	size_t secondCopy = secondCopyNecessary ? size - firstCopy : 0;
	
	std::memcpy((char*)data + firstCopy, _metadata->hostBegin, secondCopy);
	
	return secondCopyNecessary ? secondCopy : tail + firstCopy;
}

__device__ HostReflection::DeviceQueue::DeviceQueue(QueueMetaData* m)
: _metadata(m)
{
	std::printf("binding device queue to metadata (%d size, "
		"%d head, %d tail, %d mutex)\n", m->size, m->head, m->tail, m->mutex);
}

__device__ HostReflection::DeviceQueue::~DeviceQueue()
{

}

__device__ bool HostReflection::DeviceQueue::push(const void* data, size_t size)
{
	if(size > _capacity()) return false;

	if(!_lock()) return false;	

	std::printf("pushing %d bytes into gpu->cpu queue.\n", size);

	size_t end  = _metadata->size;
	size_t head = _metadata->head;

	size_t remainder = end - head;
	size_t firstCopy = min(remainder, size);

	std::memcpy(_metadata->deviceBegin + head, data, firstCopy);

	bool secondCopyNecessary = firstCopy != size;

	size_t secondCopy = secondCopyNecessary ? size - firstCopy : 0;
	
	std::memcpy(_metadata->deviceBegin, (char*)data + firstCopy, secondCopy);
	_metadata->head = secondCopyNecessary ? secondCopy : head + firstCopy;

	_unlock();
	
	return true;
}

__device__ bool HostReflection::DeviceQueue::pull(void* data, size_t size)
{
	device_assert(size <= _used());

	if(!_lock()) return false;
	
	_metadata->tail = _read(data, size);

	_unlock();
	
	return true;
}

__device__ bool HostReflection::DeviceQueue::peek()
{
	if(!_lock()) return false;
	
	Header header;
	
	_read(&header, sizeof(Header));
	
	_unlock();
	
	return header.threadId == threadId();
}

__device__ size_t HostReflection::DeviceQueue::size() const
{
	return _metadata->size;
}

__device__  size_t HostReflection::DeviceQueue::_used() const
{
	size_t end  = _metadata->size;
	size_t head = _metadata->head;
	size_t tail = _metadata->tail;
	
	size_t greaterOrEqual = head - tail;
	size_t less           = (tail) + (end - head);
	
	bool isGreaterOrEqual = head >= tail;
	
	return (isGreaterOrEqual) ? greaterOrEqual : less;
}

__device__  size_t HostReflection::DeviceQueue::_capacity() const
{
	return size() - _used();
}

__device__ bool HostReflection::DeviceQueue::_lock()
{
	device_assert(_metadata->mutex != threadId());
	
	size_t result = atomicCAS(&_metadata->mutex, (size_t)-1, threadId());
	
	return result == (size_t)-1;
}

__device__ void HostReflection::DeviceQueue::_unlock()
{
	device_assert(_metadata->mutex == threadId());
	
	_metadata->mutex = (size_t)-1;
}

__device__ size_t HostReflection::DeviceQueue::_read(
	void* data, size_t size)
{
	size_t end  = _metadata->size;
	size_t tail = _metadata->tail;

	size_t remainder = end - tail;
	size_t firstCopy = min(remainder, size);

	std::memcpy(data, _metadata->deviceBegin + tail, firstCopy);

	bool secondCopyNecessary = firstCopy != size;

	size_t secondCopy = secondCopyNecessary ? size - firstCopy : 0;
	
	std::memcpy((char*)data + firstCopy, _metadata->deviceBegin, secondCopy);
	
	return secondCopyNecessary ? secondCopy : tail + firstCopy;
}

__global__ void _bootupHostReflection(
	HostReflection::QueueMetaData* hostToDeviceMetadata,
	HostReflection::QueueMetaData* deviceToHostMetadata)
{
	_hostToDevice = new HostReflection::DeviceQueue(hostToDeviceMetadata);
	_deviceToHost = new HostReflection::DeviceQueue(deviceToHostMetadata);
}

__host__ void HostReflection::BootUp::_addMessageHandlers()
{
	addHandler(OpenFileMessageHandler,     handleOpenFile);
	addHandler(TeardownFileMessageHandler, handleTeardownFile);
	addHandler(FileWriteMessageHandler,    handleFileWrite);
	addHandler(FileReadMessageHandler,     handleFileRead);
}

__host__ HostReflection::BootUp::BootUp()
{
	report("Booting up host reflection...");

	// add message handlers
	_addMessageHandlers();

	// allocate memory for the queue
	size_t queueDataSize = HostReflection::maxMessageSize() * 2;
	size_t size = 2 * (queueDataSize + sizeof(QueueMetaData));

	_deviceHostSharedMemory = new char[size];

	// setup the queue meta data
	QueueMetaData* hostToDeviceMetaData =
		(QueueMetaData*)_deviceHostSharedMemory;
	QueueMetaData* deviceToHostMetaData =
		(QueueMetaData*)_deviceHostSharedMemory + 1;

	char* hostToDeviceData = _deviceHostSharedMemory +
		2 * sizeof(QueueMetaData);
	char* deviceToHostData = _deviceHostSharedMemory +
		2 * sizeof(QueueMetaData) + queueDataSize;

	hostToDeviceMetaData->hostBegin = hostToDeviceData;
	hostToDeviceMetaData->size      = queueDataSize;
	hostToDeviceMetaData->head      = 0;
	hostToDeviceMetaData->tail      = 0;
	hostToDeviceMetaData->mutex     = (size_t)-1;

	deviceToHostMetaData->hostBegin = deviceToHostData;
	deviceToHostMetaData->size      = queueDataSize;
	deviceToHostMetaData->head      = 0;
	deviceToHostMetaData->tail      = 0;
	deviceToHostMetaData->mutex     = (size_t)-1;

	// Allocate the queues
	_hostToDeviceQueue = new HostQueue(hostToDeviceMetaData);
	_deviceToHostQueue = new HostQueue(deviceToHostMetaData);

	// Map the memory onto the device
	cudaHostRegister(_deviceHostSharedMemory, size, 0);

	char* devicePointer = 0;
	
	cudaHostGetDevicePointer(&devicePointer,
		_deviceHostSharedMemory, 0);

	// Send the metadata to the device
	QueueMetaData* hostToDeviceMetaDataPointer =
		(QueueMetaData*)devicePointer;
	QueueMetaData* deviceToHostMetaDataPointer =
		(QueueMetaData*)devicePointer + 1;

	hostToDeviceMetaData->deviceBegin = devicePointer +
		2 * sizeof(QueueMetaData);
	deviceToHostMetaData->deviceBegin = devicePointer +
		2 * sizeof(QueueMetaData) + queueDataSize;

	_bootupHostReflection<<<1, 1>>>(hostToDeviceMetaDataPointer,
		deviceToHostMetaDataPointer);

	// start up the host worker thread
	_kill   = false;
	_thread = new boost::thread(_runThread, this);
}

__global__ void _teardownHostReflection()
{
	delete _hostToDevice;
	delete _deviceToHost;
}

__host__ HostReflection::BootUp::~BootUp()
{
	report("Destroying host reflection");
	// kill the thread
	_kill = true;
	_thread->join();
	delete _thread;
	
	// destroy the device queues
	_teardownHostReflection<<<1, 1>>>();
	
	// destroy the host queues
	delete _hostToDeviceQueue;
	delete _deviceToHostQueue;
	
	// delete the queue memory
	delete[] _deviceHostSharedMemory;
}

__host__ void HostReflection::BootUp::addHandler(int handlerId,
	MessageHandler handler)
{
	assert(_handlers.count(handlerId) == 0);
	_handlers.insert(std::make_pair(handlerId, handler));
}

__host__ bool HostReflection::BootUp::_handleMessage()
{
	if(!_deviceToHostQueue->peek())
	{
		return false;
	}
	
	report("  found message in gpu->cpu queue, pulling it...");
	
	Header header;
	
	_deviceToHostQueue->pull(&header, sizeof(Header));

	report("   type     " << header.type);
	report("   threadId " << header.threadId);
	report("   size     " << header.size);
	report("   handler  " << header.handler);
	
	HandlerMap::iterator handler = _handlers.find(header.handler);
	assert(handler != _handlers.end());
	
	Message* message = reinterpret_cast<Message*>(new char[header.size]);
	
	_deviceToHostQueue->pull(message, header.size);
	
	handler->second(message);
	
	delete[] reinterpret_cast<char*>(message);
	
	return true;
}

__host__ void HostReflection::BootUp::_run()
{
	report(" Host reflection worker thread started.");

	while(!_kill)
	{
		if(!_handleMessage())
		{
			boost::thread::yield();
		}
	}
}

__host__ void HostReflection::BootUp::_runThread(BootUp* booter)
{
	booter->_run();
}

HostReflection::BootUp* HostReflection::_booter = 0;

}

