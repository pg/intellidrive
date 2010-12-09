package com.intellidrive.iscsi;

import java.util.Arrays;

import org.jscsi.layer.block.BlockDeviceController;
import org.jscsi.layer.block.exceptions.BlockCorruptException;
import org.jscsi.layer.block.exceptions.BlockIOException;
import org.jscsi.layer.block.exceptions.BlockTransactionException;

public class MemoryBlockDeviceController extends BlockDeviceController
{
   private static final int DEFAULT_BLOCK_SIZE = 4096;
   
   private static final int DEFAULT_BLOCK_COUNT = 
      512 * 1024 * 1024 / DEFAULT_BLOCK_SIZE;

   private final int blockSize;
   private final int numBlocks;
   
   private byte[] data;
   
   public MemoryBlockDeviceController()
   {
      this(DEFAULT_BLOCK_SIZE, DEFAULT_BLOCK_COUNT);
   }
   
   public MemoryBlockDeviceController(int blockSize, int numBlocks)
   {
      this.blockSize = blockSize;
      this.numBlocks = numBlocks;
      
      this.data = new byte[numBlocks * blockSize];
   }
   
   @Override
   public void startup()
   {         
   }

   @Override
   public void shutdown()
   {
   }

   @Override
   public byte[] readBlocks(long firstBlock, int numBlocks)
         throws BlockIOException, BlockCorruptException
   {
      int startIndex = (int) firstBlock * blockSize; 
      return Arrays.copyOfRange(
         data, 
         startIndex, 
         startIndex + numBlocks*blockSize);
   }

   @Override
   public void writeBlocks(long firstBlock, int numBlocks, byte[] data)
         throws BlockIOException, BlockTransactionException
   {
      System.arraycopy(
         data, 
         0, 
         this.data, 
         (int)firstBlock*blockSize, 
         numBlocks * blockSize);
   }

   @Override
   public long getDeviceSize()
   {
      return data.length;
   }

   @Override
   public int getBlockSize()
   {
      return blockSize;
   }

   @Override
   public long getNumBlocks()
   {
      return numBlocks;
   }      
}
