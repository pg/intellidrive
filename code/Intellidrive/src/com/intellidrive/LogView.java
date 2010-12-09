package com.intellidrive;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Iterator;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.Timer;

import com.intellidrive.optimizer.AccessLog;
import com.intellidrive.optimizer.AccessLog.AccessLogEntry;

public class LogView extends JPanel implements ActionListener
{
   private static final long serialVersionUID = 1L;
   
   private static final int VIEW_WIDTH = 400;
   private static final int VIEW_HEIGHT = 50;
   private static final int BORDER_SIZE = 5;
   
   private int blockCount = 512 * (1024 * 1024) / 4096;
   
   private AccessLog log;
   private Iterator<AccessLogEntry> entryIt;
   
   public static void main(String[] args) throws FileNotFoundException, IOException 
   {     
      JFrame frame = new JFrame("Intellidrive: LogView");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      LogView logView = new LogView();
      
      logView.setBackground(Color.WHITE);
      
      frame.add(logView);
      frame.setSize(VIEW_WIDTH + 2 * BORDER_SIZE + 15, 2 * BORDER_SIZE + 85);

      Dimension scrn = Toolkit.getDefaultToolkit().getScreenSize();
      frame.setBounds( (scrn.width-frame.getWidth())/2, (scrn.height-frame.getHeight())/2, frame.getWidth(), frame.getHeight() );
      
      frame.setVisible(true);
   }

   public LogView() throws FileNotFoundException, IOException
   {
      log = AccessLog.load(new FileInputStream("log"));
      
      float randomAccessCount = 0;
      float sequentialAccessCount = 0;
      
      int previousBlock = -100;
      for(AccessLogEntry entry : log.getLog())
      {
         if(entry.getBlockNumber() != previousBlock + 1)
         {
            randomAccessCount++;
         } else
         {
            sequentialAccessCount++;
         }
         
         previousBlock = entry.getBlockNumber();
      }
      
      System.out.println("Loaded " + log.getLog().size() + " log entries");
      System.out.println((randomAccessCount/log.getLog().size() * 100) + "% random");
      System.out.println((sequentialAccessCount/log.getLog().size() * 100) + "% sequential");
      
      entryIt = log.getLog().iterator();
            
      Timer timer = new Timer(1, this);
      timer.start();
   }
   
   public void paintComponent(Graphics g)
   {
      super.paintComponent(g);
      
      this.setBackground(Color.GRAY);
      
      g.setColor(Color.LIGHT_GRAY);
      g.fillRect(BORDER_SIZE, BORDER_SIZE, VIEW_WIDTH, VIEW_HEIGHT);

      g.setColor(new Color(255, 0, 0));

      if(entryIt.hasNext())
      {
         AccessLogEntry entry = entryIt.next();
         drawBlock(entry.getBlockNumber(), g);            
      }

   }
   
   private void drawBlock(int blockNumber, Graphics g)
   {
      int x = (int)(((float)blockNumber / blockCount) * (VIEW_WIDTH));
      g.drawLine(x + BORDER_SIZE, BORDER_SIZE, x + BORDER_SIZE, VIEW_HEIGHT + BORDER_SIZE - 1);             
   }

   @Override
   public void actionPerformed(ActionEvent e)
   {      
      this.repaint();
   }
}


