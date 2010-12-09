/* ----------------------------------------------------------------------------
  genetic.h
  gebhard2 april 17, 2009

  DESCRIPTION:
   Heavily influenced by example programs from the GAlib library.  We're
   using GA2DArrayGenome and GAIncrementalGA.
---------------------------------------------------------------------------- */

#include <stdlib.h>
#include <ga/ga.h>
#include <ga/GA2DArrayGenome.h>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>

namespace io = boost::iostreams;

class GeneticDriveOptimizer
{

public:
	static GeneticDriveOptimizer* GetInstance();
  float optimize(io::stream_buffer<io::file_sink>, io::stream_buffer<io::file_sink>, io::stream_buffer<io::file_sink>);

private:  // member variables
  static const unsigned int POPULATION_SIZE = 500;
  static const unsigned int NUM_GENERATIONS = 100;
  static const float PROB_MUTATION = 0.001;
  static const float PROB_CROSSOVER = 0.6;
  
  static const unsigned int IDLE_WAIT_TIME = 5;
  static const unsigned int PRECACHE_BLOCK_CAPACITY = 65536;
  static const unsigned int PRECACHE_BLOCKS_PER_SECOND = 5120;  
  static const unsigned int MIN_PRECACHE_GROUP_SIZE = 64;
  static const unsigned int MAX_PRECACHE_GROUP_SIZE = PRECACHE_BLOCK_CAPACITY;  
  static const float ACCESS_FITNESS_WEIGHT = .75f;
  static const float SIMILARITY_FITNESS_WEIGHT = .20f;
  static const float LOGICAL_SIMILARITY_FITNESS_WEIGHT = 0.5f;
  
  static const unsigned long maxExecutionTime = 60 * 60;
  
	static GeneticDriveOptimizer* s_instance;
	
  GAIncrementalGA ga;
	
  //int[] blockMap;
  //bool[] precacheBoundaryMap;

private:  // member methods

  /**
   * Constructor/Destructor
   */
	GeneticDriveOptimizer();
	~GeneticDriveOptimizer();
	
  /**
   * Defines the objective function of the genetic algorithm being used
   * @param genetic algorithm genome
   * @return objective value
   */
  //float objective(GAGenome &);

	 /**
   * Creates an initial population of random organisms
   * @param currentBlockMap
   * @return
   */
  //void createInitialPopulation(BlockMap currentBlockMap);

  /**
   * Calculates the fitness of the given organism based its estimated 
   * efficiency in processing the requests in the given access log
   * 
   * @param organism
   * @param log
   * @return
   */
  //long calculateAccessFitness(Organism organism, AccessLog log);

  /*
   * Executes a single iteration which produces a new generation of organisms
   * based on the given population
   */
  //void executeIteration(Population population, List<File> accessLogFiles, BlockMap currentBlockMap);

  /**
   * Calculates the fitness of the given organism based on its similarity to 
   * the given block map
   * 
   * @param organism
   * @param log
   * @return
   */
  //float calculateSimilarityFitness(Organism organism, BlockMap blockMap);

  /**
   * Calculates the fitness of the given organism based on its similarity to 
   * the logical block ordering
   * 
   * @param organism
   * @param log
   * @return
   */
  //float calculateLogicalSimilarityFitness(Organism organism);

  /**
   * Selects an organism from a population with a probability proportional to 
   * its fitness
   * 
   * @param population
   * @return
   */
  //Organism selectOrganism(Population population);

  /**
   * Mates two parent organisms to produce a child organism
   * 
   * @param parent1
   * @param parent2
   * @return
   */
  //Organism mate(Organism parent1, Organism parent2);
};

