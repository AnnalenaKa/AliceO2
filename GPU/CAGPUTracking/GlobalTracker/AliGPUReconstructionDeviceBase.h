#ifndef ALIGPURECONSTRUCTIONDEVICEBASE_H
#define ALIGPURECONSTRUCTIONDEVICEBASE_H

#include "AliGPUReconstructionCPU.h"
#include <array>

#if !(defined(__CINT__) || defined(__ROOTCINT__) || defined(__CLING__) || defined(__ROOTCLING__) || defined(G__ROOT))
extern template class AliGPUReconstructionKernels<AliGPUReconstructionCPUBackend>;
#endif

class AliGPUReconstructionDeviceBase : public AliGPUReconstructionCPU
{
public:
	virtual ~AliGPUReconstructionDeviceBase() override;

	char* MergerHostMemory() const {return((char*) fGPUMergerHostMemory);}
	const AliGPUCAParam* DeviceParam() const {return &mDeviceConstantMem->param;}
	virtual int DoTRDGPUTracking() override;
	virtual int GetMaxThreads() override;

protected:
	AliGPUReconstructionDeviceBase(const AliGPUCASettingsProcessing& cfg);
	AliGPUCAConstantMem mGPUReconstructors;
	void* fGPUMergerHostMemory = nullptr;
	AliGPUCAConstantMem* mDeviceConstantMem = nullptr;
    
#ifdef GPUCA_ENABLE_GPU_TRACKER
	virtual int RunTPCTrackingSlices() override;
	virtual int RefitMergedTracks(bool resetTimers) override;
	int RunTPCTrackingSlices_internal();

	virtual int InitDevice() override;
	virtual int InitDevice_Runtime() = 0;
	virtual int ExitDevice() override;
	virtual int ExitDevice_Runtime() = 0;

	virtual const AliGPUTPCTracker* CPUTracker(int iSlice) {return &workers()->tpcTrackers[iSlice];}

	virtual void ActivateThreadContext() = 0;
	virtual void ReleaseThreadContext() = 0;
	virtual void SynchronizeGPU() = 0;
	virtual void SynchronizeStream(int stream) = 0;
	virtual void SynchronizeEvents(deviceEvent* evList, int nEvents = 1) = 0;
	virtual int IsEventDone(deviceEvent* evList, int nEvents = 1) = 0;
	
	virtual int PrepareTextures();
	virtual int DoStuckProtection(int stream, void* event);
	virtual int PrepareProfile();
	virtual int DoProfile();
	
	virtual int GPUDebug(const char* state = "UNKNOWN", int stream = -1) override = 0;
	virtual void TransferMemoryInternal(AliGPUMemoryResource* res, int stream, deviceEvent* ev, deviceEvent* evList, int nEvents, bool toGPU, void* src, void* dst) override = 0;
	virtual void WriteToConstantMemory(size_t offset, const void* src, size_t size, int stream = -1, deviceEvent* ev = nullptr) override = 0;
	virtual void ReleaseEvent(deviceEvent* ev) = 0;
	virtual void RecordMarker(deviceEvent* ev, int stream) = 0;
	
	struct helperParam
	{
		void* fThreadId;
		AliGPUReconstructionDeviceBase* fCls;
		int fNum;
		void* fMutex;
		char fTerminate;
		int fPhase;
		volatile int fDone;
		volatile char fError;
		volatile char fReset;
	};
	
	struct AliGPUProcessorWorkers : public AliGPUProcessor
	{
		AliGPUCAWorkers* mWorkersProc = nullptr;
		TPCFastTransform* fTpcTransform = nullptr;
		char* fTpcTransformBuffer = nullptr;
		o2::trd::TRDGeometryFlat* fTrdGeometry = nullptr;
		void* SetPointersDeviceProcessor(void* mem);
		void* SetPointersFlatObjects(void* mem);
		short mMemoryResWorkers = -1;
		short mMemoryResFlat = -1;
	};
	
	template <class T> struct eventStruct
	{
		T selector[NSLICES];
		T stream[GPUCA_GPU_MAX_STREAMS];
		T init;
		T constructor;
	};
	
	int PrepareFlatObjects();

	int ReadEvent(int iSlice, int threadId);
	void WriteOutput(int iSlice, int threadId);
	int GlobalTracking(int iSlice, int threadId, helperParam* hParam);

	int StartHelperThreads();
	int StopHelperThreads();
	void ResetHelperThreads(int helpers);
	void ResetThisHelperThread(helperParam* par);

	int GetThread();
	void ReleaseGlobalLock(void* sem);

	static void* helperWrapper(void*);
	
	AliGPUProcessorWorkers mProcShadow; //Host copy of tracker objects that will be used on the GPU
	AliGPUProcessorWorkers mProcDevice; //tracker objects that will be used on the GPU
	AliGPUCAWorkers* &mWorkersShadow = mProcShadow.mWorkersProc;
	AliGPUCAWorkers* &mWorkersDevice = mProcDevice.mWorkersProc;

	int fThreadId = -1; //Thread ID that is valid for the local CUDA context
    int fDeviceId = -1; //Device ID used by backend

	unsigned int fBlockCount = 0;                 //Default GPU block count
	unsigned int fThreadCount = 0;                //Default GPU thread count
	unsigned int fConstructorBlockCount = 0;      //GPU blocks used in Tracklet Constructor
	unsigned int fSelectorBlockCount = 0;         //GPU blocks used in Tracklet Selector
	unsigned int fConstructorThreadCount = 0;
	unsigned int fSelectorThreadCount = 0;
	unsigned int fFinderThreadCount = 0;
	unsigned int fTRDThreadCount = 0;

#ifdef GPUCA_GPU_TIME_PROFILE
	unsigned long long int fProfTimeC, fProfTimeD; //Timing
#endif

	helperParam* fHelperParams = nullptr; //Control Struct for helper threads
	void* fHelperMemMutex = nullptr;

#ifdef __ROOT__
#define volatile
#endif
	volatile int fSliceOutputReady;
	volatile char fSliceLeftGlobalReady[NSLICES];
	volatile char fSliceRightGlobalReady[NSLICES];
#ifdef __ROOT__
#undef volatile
#endif
	void* fSliceGlobalMutexes = nullptr;
	std::array<char, NSLICES> fGlobalTrackingDone;
	std::array<char, NSLICES> fWriteOutputDone;

	int fNSlaveThreads = 0;	//Number of slave threads currently active

	int fGPUStuck = 0;		//Marks that the GPU is stuck, skip future events
	
	int mNStreams = 0;
	eventStruct<void*> mEvents;
	bool mStreamInit[GPUCA_GPU_MAX_STREAMS] = {false};
#endif
};

#endif
