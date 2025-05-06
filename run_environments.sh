#!/bin/bash

# Function to run simulation with different environments
run_simulation() {
    local scenario=$1
    local output_dir="Output/trial1Output_${scenario}"
    
    # Create output directory if it doesn't exist
    mkdir -p $output_dir
    
    # Run the simulation with the specified scenario
    ./ns3 run "ran-simulator --scenario=$scenario --outputDir=$output_dir"
}

# Run simulations for different environments
echo "Running simulation for Rural Macro (RMa) environment..."
run_simulation "RMa_LoS"

echo "Running simulation for Urban Macro (UMa) environment..."
run_simulation "UMa_LoS"

echo "Running simulation for Urban Micro (UMi) environment..."
run_simulation "UMi_StreetCanyon"

echo "All simulations completed!" 