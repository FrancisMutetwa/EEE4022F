import sys
import os
import subprocess
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                           QHBoxLayout, QPushButton, QLabel, QSpinBox, 
                           QComboBox, QTextEdit, QMessageBox, QTabWidget)
from PyQt5.QtCore import Qt, QProcess
from network_visualizer import NetworkVisualizer

class SimulationGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("RAN Simulator Control Panel")
        self.setGeometry(100, 100, 1200, 800)
        
        # Initialize the simulation process
        self.simulation_process = None
        
        # Create the main widget and layout
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        
        # Create tab widget
        self.tabs = QTabWidget()
        layout.addWidget(self.tabs)
        
        # Create simulation control tab
        control_tab = QWidget()
        control_layout = QVBoxLayout(control_tab)
        
        # Create simulation parameters section
        params_group = QWidget()
        params_layout = QVBoxLayout(params_group)
        
        # Simulation duration
        duration_layout = QHBoxLayout()
        duration_layout.addWidget(QLabel("Simulation Duration (s):"))
        self.duration_spinbox = QSpinBox()
        self.duration_spinbox.setRange(1, 3600)
        self.duration_spinbox.setValue(600)
        duration_layout.addWidget(self.duration_spinbox)
        params_layout.addLayout(duration_layout)
        
        # Round duration
        round_layout = QHBoxLayout()
        round_layout.addWidget(QLabel("Round Duration (ms):"))
        self.round_spinbox = QSpinBox()
        self.round_spinbox.setRange(1000, 60000)
        self.round_spinbox.setValue(10000)
        round_layout.addWidget(self.round_spinbox)
        params_layout.addLayout(round_layout)
        
        # UE distribution
        distribution_layout = QHBoxLayout()
        distribution_layout.addWidget(QLabel("UE Distribution:"))
        self.distribution_combo = QComboBox()
        self.distribution_combo.addItems(["uniform", "random"])
        distribution_layout.addWidget(self.distribution_combo)
        params_layout.addLayout(distribution_layout)
        
        control_layout.addWidget(params_group)
        
        # Create control buttons
        buttons_layout = QHBoxLayout()
        
        self.start_button = QPushButton("Start Simulation")
        self.start_button.clicked.connect(self.start_simulation)
        buttons_layout.addWidget(self.start_button)
        
        self.stop_button = QPushButton("Stop Simulation")
        self.stop_button.clicked.connect(self.stop_simulation)
        self.stop_button.setEnabled(False)
        buttons_layout.addWidget(self.stop_button)
        
        control_layout.addLayout(buttons_layout)
        
        # Create output display
        self.output_text = QTextEdit()
        self.output_text.setReadOnly(True)
        control_layout.addWidget(self.output_text)
        
        # Add control tab
        self.tabs.addTab(control_tab, "Simulation Control")
        
        # Create visualization tab
        self.visualization_tab = QWidget()
        self.visualization_layout = QVBoxLayout(self.visualization_tab)
        self.tabs.addTab(self.visualization_tab, "Network Visualization")
        
        # Initialize visualizer
        self.visualizer = None
    
    def start_simulation(self):
        self.output_text.clear()
        self.output_text.append("Starting simulation...")
        
        # Disable start button during simulation
        self.start_button.setEnabled(False)
        self.stop_button.setEnabled(True)
        
        # Get simulation parameters
        sim_duration = self.duration_spinbox.value()
        round_duration = self.round_spinbox.value()
        ue_distribution = self.distribution_combo.currentText().lower()
        
        # Get absolute path to ns-3 directory
        current_dir = os.path.dirname(os.path.abspath(__file__))
        ns3_dir = os.path.join(os.path.dirname(current_dir), "ns-3.36")
        
        if not os.path.exists(ns3_dir):
            self.output_text.append(f"Error: ns-3 directory not found at {ns3_dir}")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
            return
        
        # Check if ns3 script exists and is executable
        ns3_script = os.path.join(ns3_dir, "ns3")
        if not os.path.exists(ns3_script):
            self.output_text.append(f"Error: ns3 script not found at {ns3_script}")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
            return
        
        if not os.access(ns3_script, os.X_OK):
            self.output_text.append(f"Error: ns3 script is not executable. Please run 'chmod +x {ns3_script}'")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
            return
        
        # Create QProcess for building
        self.build_process = QProcess()
        self.build_process.setWorkingDirectory(ns3_dir)
        
        # Set up environment variables
        env = self.build_process.processEnvironment()
        env.insert("PATH", os.environ.get("PATH", ""))
        self.build_process.setProcessEnvironment(env)
        
        # Connect signals
        self.build_process.readyReadStandardOutput.connect(
            lambda: self.output_text.append(str(self.build_process.readAllStandardOutput(), 'utf-8').strip())
        )
        self.build_process.readyReadStandardError.connect(
            lambda: self.output_text.append(f"Error: {str(self.build_process.readAllStandardError(), 'utf-8').strip()}")
        )
        self.build_process.finished.connect(self.on_build_finished)
        
        # Build command
        build_cmd = "/bin/bash"
        build_args = ["-c", f"cd \"{ns3_dir}\" && ./ns3 build"]
        self.output_text.append(f"Building simulation: {' '.join([build_cmd] + build_args)}")
        
        try:
            self.build_process.start(build_cmd, build_args)
            if not self.build_process.waitForStarted(5000):
                self.output_text.append("Error: Build process failed to start")
                self.start_button.setEnabled(True)
                self.stop_button.setEnabled(False)
        except Exception as e:
            self.output_text.append(f"Error starting build process: {str(e)}")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
    
    def on_build_finished(self, exit_code, exit_status):
        if exit_code != 0:
            self.output_text.append("Build failed!")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
            return
        
        self.output_text.append("Build completed successfully. Starting simulation...")
        
        # Get simulation parameters
        sim_duration = self.duration_spinbox.value()
        round_duration = self.round_spinbox.value()
        ue_distribution = self.distribution_combo.currentText().lower()
        
        # Get absolute path to ns-3 directory
        current_dir = os.path.dirname(os.path.abspath(__file__))
        ns3_dir = os.path.join(os.path.dirname(current_dir), "ns-3.36")
        
        # Create QProcess for simulation
        self.sim_process = QProcess()
        self.sim_process.setWorkingDirectory(ns3_dir)
        
        # Set up environment variables
        env = self.sim_process.processEnvironment()
        env.insert("PATH", os.environ.get("PATH", ""))
        self.sim_process.setProcessEnvironment(env)
        
        # Connect signals
        self.sim_process.readyReadStandardOutput.connect(
            lambda: self.output_text.append(str(self.sim_process.readAllStandardOutput(), 'utf-8').strip())
        )
        self.sim_process.readyReadStandardError.connect(
            lambda: self.output_text.append(f"Error: {str(self.sim_process.readAllStandardError(), 'utf-8').strip()}")
        )
        self.sim_process.finished.connect(self.simulation_finished)
        
        # Run command
        run_cmd = "/bin/bash"
        run_args = ["-c", f"cd \"{ns3_dir}\" && ./ns3 run \"src/nr/examples/ran-simulator\" -- --simDurationS={sim_duration} --simRoundDurationMs={round_duration} --spread={ue_distribution}"]
        self.output_text.append(f"Running simulation: {' '.join([run_cmd] + run_args)}")
        
        try:
            self.sim_process.start(run_cmd, run_args)
            if not self.sim_process.waitForStarted(5000):
                self.output_text.append("Error: Simulation process failed to start")
                self.start_button.setEnabled(True)
                self.stop_button.setEnabled(False)
        except Exception as e:
            self.output_text.append(f"Error starting simulation process: {str(e)}")
            self.start_button.setEnabled(True)
            self.stop_button.setEnabled(False)
    
    def stop_simulation(self):
        if hasattr(self, 'build_process') and self.build_process.state() == QProcess.Running:
            self.build_process.kill()
            self.output_text.append("Build process terminated.")
        
        if hasattr(self, 'sim_process') and self.sim_process.state() == QProcess.Running:
            self.sim_process.kill()
            self.output_text.append("Simulation process terminated.")
        
        self.start_button.setEnabled(True)
        self.stop_button.setEnabled(False)
    
    def simulation_finished(self, exit_code, exit_status):
        if exit_code != 0:
            self.output_text.append("Simulation failed!")
        else:
            self.output_text.append("Simulation completed successfully!")
            
            # Try to load and visualize results
            try:
                # Get the path to the simulation output directory
                current_dir = os.path.dirname(os.path.abspath(__file__))
                output_dir = os.path.join(os.path.dirname(current_dir), "Output", "trial2Output")
                
                # Check if output files exist
                required_files = ["node_positions.csv"]
                
                missing_files = [f for f in required_files if not os.path.exists(os.path.join(output_dir, f))]
                if missing_files:
                    self.output_text.append(f"Error: Missing required output files: {', '.join(missing_files)}")
                    return
                
                # Clear previous visualization if any
                for i in reversed(range(self.visualization_layout.count())): 
                    self.visualization_layout.itemAt(i).widget().setParent(None)
                
                # Create new visualization
                self.visualizer = NetworkVisualizer(output_dir=output_dir)
                
                # Add the visualization to the tab
                self.visualization_layout.addWidget(self.visualizer)
                self.output_text.append("Visualization updated with new simulation data.")
                
            except Exception as e:
                self.output_text.append(f"Error loading visualization data: {str(e)}")
        
        self.start_button.setEnabled(True)
        self.stop_button.setEnabled(False)

def main():
    app = QApplication(sys.argv)
    window = SimulationGUI()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
