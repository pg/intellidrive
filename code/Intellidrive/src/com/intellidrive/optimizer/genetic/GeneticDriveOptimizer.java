package com.intellidrive.optimizer.genetic;

import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

import com.intellidrive.optimizer.AccessLog;
import com.intellidrive.optimizer.BlockMap;
import com.intellidrive.optimizer.DriveOptimizer;
import com.intellidrive.optimizer.PrecacheBoundaryMap;
import com.intellidrive.optimizer.AccessLog.AccessLogEntry;
import com.intellidrive.optimizer.AccessLog.AccessType;

public class GeneticDriveOptimizer implements DriveOptimizer
{
   private int POPULATION_SIZE = 128;
   
   private int IDLE_WAIT_TIME = 5;
   private int PRECACHE_BLOCK_CAPACITY = 65536;
   private int PRECACHE_BLOCKS_PER_SECOND = 5120; 
   
   private int MIN_PRECACHE_GROUP_SIZE = 32;
   private int MAX_PRECACHE_GROUP_SIZE = 512;
   
   private float ACCESS_FITNESS_WEIGHT = .75f;
   private float SIMILARITY_FITNESS_WEIGHT = .20f;
   private float LOGICAL_SIMILARITY_FITNESS_WEIGHT = 0.5f;
   
   private PrintWriter outFile;
   private long maxExecutionTime = 60 * 60;
   
   @Override
   public OptimizerResult optimize(
      BlockMap currentBlockMap,
      PrecacheBoundaryMap currentPrecacheBoundaryMap,
      List<AccessLog> accessLogs) throws IOException
   {
      System.out.println("Creating initial population");
      
      Population population = createInitialPopulation(currentBlockMap);
      
      long startTime = System.currentTimeMillis();

      int iteration = 1;
      
      outFile = new PrintWriter(new FileWriter("ga.log"));
      System.out.println("Starting optimization ...");
      System.out.println();
      
      do
      {
         Population newGeneration = executeIteration(
            population, accessLogs, currentBlockMap);
                  
         // TODO: Check for stopping criteria
      
         outFile.println(population.getBestOrganism().getFitness());
         outFile.flush();
         
         System.out.println("Completed iteration #" + iteration);         
         System.out.println("Fitness of best organism: " + population.getBestOrganism().getFitness());
         System.out.println();
         
         iteration++;
         
         population = newGeneration;

      } while(System.currentTimeMillis() - startTime < maxExecutionTime * 1000);

      Organism bestOrganism = population.getBestOrganism();
      return new OptimizerResult(bestOrganism.getBlockMap(), bestOrganism
            .getPrecacheBoundaryMap());
   }

   /**
    * Creates an initial population of random organisms
    * @param currentBlockMap
    * @return
    */
   private Population createInitialPopulation(BlockMap currentBlockMap)
   {
      Population organisms = new Population();
      
      List<Integer> blocks = new ArrayList<Integer>();
      for(int blockNumber = 0; blockNumber < currentBlockMap.size(); blockNumber++)
      {
         blocks.add(blockNumber);
      }
      
      Random random = new Random();
      for(int cnt = 1; cnt <= POPULATION_SIZE; cnt++)
      {  
         Collections.shuffle(blocks);
         
         /*
         if(random.nextBoolean())
         {
            Collections.shuffle(blocks);
         } else
         {
            Collections.sort(blocks);
         }
         */
         
         int[] blockMap = new int[blocks.size()];
         
         for(int idx = 0; idx < blockMap.length; idx++)
         {
            blockMap[idx] = blocks.get(idx);
         }

         int blockNumber = 0;
         boolean[] precacheBoundaryMap = new boolean[blocks.size()];
         while(blockNumber < blocks.size())
         {
            blockNumber += random.nextInt(
               MAX_PRECACHE_GROUP_SIZE - MIN_PRECACHE_GROUP_SIZE) + MIN_PRECACHE_GROUP_SIZE;
            
            if(blockNumber < precacheBoundaryMap.length)
            {
               precacheBoundaryMap[blockNumber] = true;
            }
         }
         
         Organism organism = new Organism(
            new BlockMap(blockMap), 
            new PrecacheBoundaryMap(precacheBoundaryMap));
         
         organisms.add(organism);
      }
      
      return organisms;
   }
   
   /*
    * Executes a single iteration which produces a new generation of organisms
    * based on the given population
    */
   private Population executeIteration(
      Population population,
      List<AccessLog> accessLogs,
      BlockMap currentBlockMap) throws IOException
   {
      // Update fitness
      
      System.out.print(" - Calculating fitness\t");

      float maxFitness = 0;
      
      for(Organism organism : population)
      {
         organism.setFitness(0);
      }
      
      for (AccessLog log : accessLogs)
      {
         for(Organism organism : population)
         {
            float fitness = organism.getFitness();
            fitness += calculateAccessFitness(organism, log);
            
            organism.setFitness(fitness);
            
            if(fitness > maxFitness)
            {
               maxFitness = fitness;
            }

            System.out.print(".");
         }
      }      

      System.out.println();
      
      // Normalize fitness
      /*
      for(Organism organism : population)
      {
         float fitness = organism.getFitness();
         fitness /= maxFitness;
         organism.setFitness(fitness);
      }
      */
      
      // Incorporate similarity to current block order into fitness
      // Incorporate similarity to logical block order into fitness
      
      /*
      for(Organism organism : population)
      {
         float accessFitness = organism.getFitness();         

         float similarityFitness = calculateSimilarityFitness(
            organism, currentBlockMap);
         
         float logicalSimilarityFitness = calculateLogicalSimilarityFitness(
            organism);
         
         organism.setFitness(
            ACCESS_FITNESS_WEIGHT * accessFitness + 
            SIMILARITY_FITNESS_WEIGHT * similarityFitness + 
            LOGICAL_SIMILARITY_FITNESS_WEIGHT * logicalSimilarityFitness);                
      }      

      */
      
      population.updateTotalFitness();
      
      System.out.print(" - Mating organisms\t");

      // Mate most fit organisms
      Population newGeneration = new Population();
      for(Organism organism : population)
      {
         Organism mate = selectOrganism(organism, population);
         
         Organism child = mate(organism, mate);
         newGeneration.add(child);
         
         System.out.print(".");
      }
      
      System.out.println();
      
      return newGeneration;
   }

   /**
    * Calculates the fitness of the given organism based its estimated 
    * efficiency in processing the requests in the given access log
    * 
    * @param organism
    * @param log
    * @return
    */
   public long calculateAccessFitness(Organism organism, AccessLog log)
   {
      BlockMap blockMap = organism.getBlockMap();
      
      double fitness = 0;

      AccessLogEntry previousEntry = null;

      for(AccessLogEntry logEntry : log.getLog())
      {
         if(logEntry.getAccessType() == AccessType.WRITE)
         {
            previousEntry = null;
            continue;
         }
         
         int blockNumber = blockMap.getPhysicalBlockNumber(logEntry.getBlockNumber());
         
         if(previousEntry != null)
         {
            int previousBlockNumber = 
               blockMap.getPhysicalBlockNumber(previousEntry.getBlockNumber());
            
            if(blockNumber == previousBlockNumber + 1)
            {
               // The read is sequential
               fitness += 1.0d;
            } else
            {
               // The read is random
               fitness += 1.0d - ((double)Math.abs(blockNumber - previousBlockNumber) / (double)blockMap.size());
            }            
         } else
         {
            // The read is random
         }

         previousEntry = logEntry;         
      }

      return (long) fitness;
   }

   /**
    * Calculates the fitness of the given organism based on its similarity to 
    * the given block map
    * 
    * @param organism
    * @param log
    * @return
    */
   private float calculateSimilarityFitness(Organism organism, BlockMap blockMap)
   {
      int matchingBlocks = 0;
      
      for(int block = 0; block < blockMap.size(); block++)
      {
         int organismBlock = organism.getBlockMap().getPhysicalBlockNumber(block);

         if(organismBlock == blockMap.getPhysicalBlockNumber(block))
         {
            matchingBlocks++;
         }
      }
      
      return (float) matchingBlocks / (float) blockMap.size();
   }

   /**
    * Calculates the fitness of the given organism based on its similarity to 
    * the logical block ordering
    * 
    * @param organism
    * @param log
    * @return
    */
   private float calculateLogicalSimilarityFitness(Organism organism)
   {
      int matchingBlocks = 0;
      
      BlockMap blockMap = organism.getBlockMap();
      
      for(int logicalBlock = 0; logicalBlock < blockMap.size(); logicalBlock++)
      {
         int organismBlock = blockMap.getPhysicalBlockNumber(logicalBlock);

         if(organismBlock == logicalBlock)
         {
            matchingBlocks++;
         }
      }
      
      return (float) matchingBlocks / (float) blockMap.size();
   }

   
   /**
    * Selects an organism from a population with a probability proportional to 
    * its fitness
    * 
    * @param population
    * @return
    */
   private Organism selectOrganism(Organism otherMate, Population population)
   {
      Random random = new Random();
      double remainingFitness = random.nextDouble() * population.getTotalFitness(); 
            
      for(Organism organism : population)
      {
         remainingFitness -= organism.getFitness();
         
         if(remainingFitness <= 0 && organism != otherMate)
         {
            return organism;
         }
      }
      
      // Choose a random organism
      return population.get(random.nextInt(population.size()));
   }
      
   /**
    * Mates two parent organisms to produce a child organism
    * 
    * @param parent1
    * @param parent2
    * @return
    */
   private Organism mate(Organism parent1, Organism parent2)
   {
      class MatingOrganism
      {
         private Organism organism;
         public int blockNumber;
         
         public MatingOrganism(Organism organism)
         {
            this.organism = organism;
            this.blockNumber = 0;
         }
         
         public int getPhysicalBlockNumber()
         {
            return organism.getBlockMap().getPhysicalBlockNumber(blockNumber);
         }
      }
      
      Random random = new Random();
      
      int blockCount = parent1.getBlockMap().size();
      int[] blockMap = new int[blockCount];
      boolean[] precacheBoundaryMap = new boolean[blockCount];
      
      boolean[] physicalBlockUsed = new boolean[blockCount];
      
      int blockNumber = 0;
      
      MatingOrganism p1 = new MatingOrganism(parent1);
      MatingOrganism p2 = new MatingOrganism(parent2);
      
      do
      {
         MatingOrganism parent = random.nextBoolean() ? p1 : p2;
         int groupSize = random.nextInt(MAX_PRECACHE_GROUP_SIZE);
         
         int cnt = 0;
         while(blockNumber < blockCount && cnt < groupSize)
         {         
            if(random.nextDouble() <= (1.0/1000.0))
            {
               // Mutation choose an unused block
               
               int b = random.nextInt(blockCount);
               while(b < blockCount && physicalBlockUsed[b])
               {
                  b++;
               }

               if(b < blockCount)
               {
                  blockMap[blockNumber] = b;
                  physicalBlockUsed[b] = true;
               
                  blockNumber++;
                  cnt++;
               }
               
               continue;
            }
            
            while(parent.blockNumber < blockCount &&
                  physicalBlockUsed[parent.getPhysicalBlockNumber()])
            {
               parent.blockNumber++;
            }
            
            if(parent.blockNumber >= blockCount)
            {
               break;
            }
            
            blockMap[blockNumber] = parent.getPhysicalBlockNumber();
            
            physicalBlockUsed[parent.getPhysicalBlockNumber()] = true;
            
            blockNumber++;
            cnt++;
         }
         
         if(blockNumber < blockCount)
         {
            precacheBoundaryMap[blockNumber] = true;
         }
      } while(blockNumber < blockCount);
      
      return new Organism(
         new BlockMap(blockMap), new PrecacheBoundaryMap(precacheBoundaryMap));
   }
   
   public static void main(String args[]) throws IOException
   {
      GeneticDriveOptimizer optimizer = new GeneticDriveOptimizer();
      
      final int blockCount = 512 * 1024 * 1024 / 4096;
      
      // Create the initial block map
      int[] blockMap = new int[blockCount];
      for(int idx = 0; idx < blockMap.length; idx++)
      {
         blockMap[idx] = idx;
      }
      
      BlockMap currentBlockMap = new BlockMap(blockMap);
      
      // Create the initial precache boundary map
      int avgGroupSize = (optimizer.MIN_PRECACHE_GROUP_SIZE + optimizer.MAX_PRECACHE_GROUP_SIZE) / 2;
      boolean[] boundaryMap = new boolean[blockCount];
      for(int idx = 0; idx < boundaryMap.length; idx++)
      {
         boundaryMap[idx] = (idx % avgGroupSize == 0) ? true : false;
      }
      
      PrecacheBoundaryMap currentPrecacheBoundaryMap = new PrecacheBoundaryMap(boundaryMap);
      
      // Create a list of log files
      List<AccessLog> accessLogs = new ArrayList<AccessLog>();
      accessLogs.add(AccessLog.load(new FileInputStream("log")));
      
      /*
      // Test fitness function
      AccessLog log = AccessLog.load(new FileInputStream("log"));
      Organism organism = new Organism(currentBlockMap, currentPrecacheBoundaryMap);      
      float fitness = optimizer.calculateAccessFitness(organism, log);
      
      System.out.println("Fitness: " + fitness);
      */
      // Start optimization
      
      optimizer.optimize(currentBlockMap, currentPrecacheBoundaryMap, accessLogs);
   }
}
