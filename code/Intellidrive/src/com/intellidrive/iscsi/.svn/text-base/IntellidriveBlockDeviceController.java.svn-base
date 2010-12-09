package com.intellidrive.iscsi;

import org.jscsi.layer.block.BlockDeviceController;
import org.jscsi.layer.block.exceptions.BlockCorruptException;
import org.jscsi.layer.block.exceptions.BlockIOException;
import org.jscsi.layer.block.exceptions.BlockTransactionException;

public class IntellidriveBlockDeviceController extends BlockDeviceController
{
   static
   {
      try
      {
         System.loadLibrary( "intellidrive" );
      } catch (UnsatisfiedLinkError e1)
      {         
         e1.printStackTrace();
      }
   }

   @Override
   public native int getBlockSize();
   
   @Override
   public native long getDeviceSize();
   
   @Override
   public native long getNumBlocks();

   @Override
   public native byte[] readBlocks(long firstBlock, int numBlocks)
         throws BlockIOException, BlockCorruptException;

   @Override
   public native void shutdown();

   @Override
   public native void startup();

   @Override
   public native void writeBlocks(long firstBlock, int numBlocks, byte[] data)
         throws BlockIOException, BlockTransactionException;
}
