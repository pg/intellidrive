package com.intellidrive.optimizer;

import java.io.InputStream;
import java.io.OutputStream;

public class BlockMap 
{
	private int[] map;

	public BlockMap(int[] map)
	{
	   this.map = map;
	}
	
	public int getPhysicalBlockNumber(int logicalBlockNumber)
	{
	   return map[logicalBlockNumber];
	}

	public int size()
	{
	   return map.length;
	}
	
	public static BlockMap load(InputStream in)
	{
	   return null;
	}
	
	public static void save(OutputStream out, BlockMap map)
	{
	}
}
