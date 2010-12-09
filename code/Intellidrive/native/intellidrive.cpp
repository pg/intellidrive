#include "com_intellidrive_iscsi_IntellidriveBlockDeviceController.h"

#include <iostream>

using namespace std;

static const int BLOCK_SIZE = 4096;
static const long DEVICE_SIZE = 128 * 1024 * 1024;

static jbyte* data;

JNIEXPORT void JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_startup
  (JNIEnv *, jobject)
{
	data = new jbyte[DEVICE_SIZE];
	cout << "Block device controller started" << endl;
}

JNIEXPORT jint JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_getBlockSize
  (JNIEnv *, jobject)
{
	return BLOCK_SIZE;
}

JNIEXPORT jlong JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_getDeviceSize
  (JNIEnv *, jobject)
{
	return DEVICE_SIZE;
}

JNIEXPORT jlong JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_getNumBlocks
  (JNIEnv *, jobject)
{
	return DEVICE_SIZE / BLOCK_SIZE;
}
  
JNIEXPORT jbyteArray JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_readBlocks
  (JNIEnv* env, jobject jobj, jlong firstBlock, jint numBlocks)
{
	cout << "Reading " << numBlocks << " blocks starting from block " << firstBlock << endl;
	
	jbyteArray blockData = env->NewByteArray(numBlocks * BLOCK_SIZE);
	env->SetByteArrayRegion(blockData, 0, numBlocks * BLOCK_SIZE, &data[firstBlock * BLOCK_SIZE]);
	
	return blockData;
}

JNIEXPORT void JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_writeBlocks
  (JNIEnv* env, jobject jobj, jlong firstBlock, jint numBlocks, jbyteArray blockData)
{
	cout << "Writing " << numBlocks << " blocks starting from block " << firstBlock << endl;

	env->GetByteArrayRegion(blockData, 0, numBlocks * BLOCK_SIZE, &data[firstBlock * BLOCK_SIZE]);
}

JNIEXPORT void JNICALL Java_com_intellidrive_iscsi_IntellidriveBlockDeviceController_shutdown
  (JNIEnv *, jobject)
{
	delete [] data;

	cout << "Block device controller shutdown" << endl;
}
