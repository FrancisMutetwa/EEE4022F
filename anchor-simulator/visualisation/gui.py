"""
Name: Francis Mutetwa
Date: 2025-05-07
Student Number: MTTFRA005
EEE4022F 2025 Final Year Project GUI submodule 

This is the main GUI for the RAN Simulator.
It allows the user to select an input trial directory, and then start the simulation.
It also allows the user to select a round number, and then load the KPI data for that round.
"""
import sys
import os
import subprocess
import shutil
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                           QHBoxLayout, QPushButton, QLabel, QSpinBox, 
                           QComboBox, QTextEdit, QMessageBox, QTabWidget,
                           QTableWidget, QTableWidgetItem, QHeaderView, QGroupBox,
                           QFileDialog)
from PyQt5.QtCore import Qt, QProcess
from PyQt5.QtGui import QFont, QColor
from network_visualizer import NetworkVisualizer

class SimulationGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("RAN Simulator Control Panel")
        self.setGeometry(100, 100, 1200, 800)
        
        # Initialize the simulation process
        self.simulation_process = None
        
        # Initialize directories
        self.input_trial_directory = None
        self.output_trial_directory = None
        
        # Create the main widget and layout
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        
        # Create trial selection section at the top
        trial_selection_layout = QHBoxLayout()
        
        # Input trial selection
        input_layout = QVBoxLayout()
        input_layout.addWidget(QLabel("Input Trial Directory:"))
        self.input_trial_label = QLabel("No input trial selected")
        self.input_trial_label.setStyleSheet("color: gray;")
        input_layout.addWidget(self.input_trial_label)
        self.select_input_button = QPushButton("Select Input Trial")
        self.select_input_button.clicked.connect(self.select_input_trial)
        input_layout.addWidget(self.select_input_button)
        trial_selection_layout.addLayout(input_layout)
        
        # Output trial display
        output_layout = QVBoxLayout()
        output_layout.addWidget(QLabel("Output Trial Directory:"))
        self.output_trial_label = QLabel("No output trial selected")
        self.output_trial_label.setStyleSheet("color: gray;")
        output_layout.addWidget(self.output_trial_label)
        trial_selection_layout.addLayout(output_layout)
        
        layout.addLayout(trial_selection_layout)
        
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
        
        # Simulation round index
        round_index_layout = QHBoxLayout()
        round_index_layout.addWidget(QLabel("Simulation Round Index:"))
        self.round_index_spinbox = QSpinBox()
        self.round_index_spinbox.setRange(0, 100)  # Setting a reasonable range
        self.round_index_spinbox.setValue(0)
        round_index_layout.addWidget(self.round_index_spinbox)
        params_layout.addLayout(round_index_layout)
        
        # Environment selection
        environment_layout = QHBoxLayout()
        environment_layout.addWidget(QLabel("Environment:"))
        self.environment_combo = QComboBox()
        self.environment_combo.addItems(["RMa_LoS", "UMa_LoS", "UMi_StreetCanyon"])
        environment_layout.addWidget(self.environment_combo)
        params_layout.addLayout(environment_layout)
        
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
        
        # Create KPI tab
        self.kpi_tab = QWidget()
        kpi_layout = QVBoxLayout(self.kpi_tab)
        
        # Add KPI controls in a group box
        kpi_controls_group = QGroupBox("KPI Data Selection")
        kpi_controls = QHBoxLayout()
        
        # Trial selection
        trial_layout = QVBoxLayout()
        trial_layout.addWidget(QLabel("Trial Number:"))
        self.trial_spinbox = QSpinBox()
        self.trial_spinbox.setRange(1, 10)
        self.trial_spinbox.setValue(1)
        trial_layout.addWidget(self.trial_spinbox)
        kpi_controls.addLayout(trial_layout)
        
        # Round selection
        round_layout = QVBoxLayout()
        round_layout.addWidget(QLabel("Round Number:"))
        self.kpi_round_spinbox = QSpinBox()
        self.kpi_round_spinbox.setRange(0, 100)
        self.kpi_round_spinbox.setValue(0)
        round_layout.addWidget(self.kpi_round_spinbox)
        kpi_controls.addLayout(round_layout)
        
        # Load button
        button_layout = QVBoxLayout()
        button_layout.addWidget(QLabel(""))  # Spacer
        self.load_kpi_button = QPushButton("Load KPI Data")
        self.load_kpi_button.clicked.connect(self.load_kpi_data)
        button_layout.addWidget(self.load_kpi_button)
        kpi_controls.addLayout(button_layout)
        
        kpi_controls_group.setLayout(kpi_controls)
        kpi_layout.addWidget(kpi_controls_group)
        
        # Create KPI tables
        self.create_kpi_tables(kpi_layout)
        
        self.tabs.addTab(self.kpi_tab, "KPI Results")
        
        # Initialize visualizer
        self.visualizer = None
    
    def create_kpi_tables(self, layout):
        # Create tables for different metric categories
        categories = [
            ("Performance Metrics", ["Average delivery time", "Jitter", "Throughput"]),
            ("Network Statistics", ["Cell occupancy", "Handovers"]),
            ("Channel Metrics", ["Satellite channel", "Terrestrial channel"])
        ]
        
        for title, metrics in categories:
            group = QGroupBox(title)
            group_layout = QVBoxLayout()
            
            table = QTableWidget()
            table.setColumnCount(2)
            table.setHorizontalHeaderLabels(["Metric", "Value"])
            table.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeToContents)
            table.horizontalHeader().setSectionResizeMode(1, QHeaderView.Stretch)
            
            # Set table properties
            table.setAlternatingRowColors(True)
            table.setStyleSheet("""
                QTableWidget {
                    gridline-color: #d3d3d3;
                    background-color: white;
                    alternate-background-color: #f6f6f6;
                }
                QHeaderView::section {
                    background-color: #f0f0f0;
                    padding: 4px;
                    border: 1px solid #d3d3d3;
                    font-weight: bold;
                }
            """)
            
            group_layout.addWidget(table)
            group.setLayout(group_layout)
            layout.addWidget(group)
            
            # Store table reference
            setattr(self, f"{title.lower().replace(' ', '_')}_table", table)

    def select_input_trial(self):
        """Open a file dialog to select the input trial directory"""
        current_dir = os.path.dirname(os.path.abspath(__file__))
        default_dir = os.path.join(os.path.dirname(current_dir), "Input")
        
        dir_path = QFileDialog.getExistingDirectory(
            self,
            "Select Input Trial Directory",
            default_dir,
            QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks
        )
        
        if dir_path:
            self.input_trial_directory = dir_path
            trial_name = os.path.basename(dir_path)
            self.input_trial_label.setText(f"Selected: {trial_name}")
            self.input_trial_label.setStyleSheet("color: black;")
            
            # Set up the corresponding output directory
            output_dir = os.path.join(os.path.dirname(current_dir), "Output", f"{trial_name}Output")
            self.output_trial_directory = output_dir
            self.output_trial_label.setText(f"Output: {os.path.basename(output_dir)}")
            self.output_trial_label.setStyleSheet("color: black;")
            
            # Update the trial number spinbox based on the directory name
            try:
                trial_num = int(trial_name.replace("trial", ""))
                self.trial_spinbox.setValue(trial_num)
            except ValueError:
                pass
            
            # Enable the simulation controls
            self.start_button.setEnabled(True)
            self.load_kpi_button.setEnabled(True)
            
            # Update the node-mobility.h file
            self.update_node_mobility_file(trial_name)

    def update_node_mobility_file(self, trial_name):
        """Update the node-mobility.h file with the selected trial"""
        try:
            current_dir = os.path.dirname(os.path.abspath(__file__))
            node_mobility_path = os.path.join(os.path.dirname(current_dir), "ns-3.36", "src", "nr", "model", "node-mobility.h")
            
            # Read the current content
            with open(node_mobility_path, 'r') as file:
                content = file.readlines()
            
            # Update the paths
            for i, line in enumerate(content):
                if "inputPath" in line:
                    content[i] = f'const string inputPath = "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/Input/{trial_name}/";\n'
                elif "outputPath" in line:
                    content[i] = f'const string outputPath = "/home/francis-mutetwa/Documents/Main FIle/trial1/anchor-simulator/Output/{trial_name}Output/";\n'
                elif "TRIAL_DIR" in line:
                    content[i] = f'#define TRIAL_DIR "{trial_name}Output"\n'
            
            # Write the updated content
            with open(node_mobility_path, 'w') as file:
                file.writelines(content)
                
            QMessageBox.information(self, "Success", f"Updated node-mobility.h to use {trial_name}")
            
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to update node-mobility.h: {str(e)}")

    def start_simulation(self):
        """Start the simulation with the selected trial directory"""
        if not self.input_trial_directory:
            QMessageBox.warning(self, "Warning", "Please select an input trial directory first!")
            return
            
        self.output_text.clear()
        self.output_text.append("Starting simulation...")
        
        # Disable start button during simulation
        self.start_button.setEnabled(False)
        self.stop_button.setEnabled(True)
        
        # Get simulation parameters
        sim_duration = self.duration_spinbox.value()
        round_duration = self.round_spinbox.value()
        sim_round_index = self.round_index_spinbox.value()
        environment = self.environment_combo.currentText()
        
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
        sim_round_index = self.round_index_spinbox.value()
        environment = self.environment_combo.currentText()
        
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
        run_args = ["-c", f"cd \"{ns3_dir}\" && ./ns3 run \"src/nr/examples/ran-simulator\" -- --simDurationS={sim_duration} --simRoundDurationMs={round_duration} --simRoundIndex={sim_round_index} --environment={environment}"]
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
                if not self.output_trial_directory:
                    self.output_text.append("Error: No output directory selected")
                    return
                
                # Check if output files exist
                required_files = ["node_positions.csv"]
                
                missing_files = [f for f in required_files if not os.path.exists(os.path.join(self.output_trial_directory, f))]
                if missing_files:
                    self.output_text.append(f"Error: Missing required output files: {', '.join(missing_files)}")
                    return
                
                # Clear previous visualization if any
                for i in reversed(range(self.visualization_layout.count())): 
                    self.visualization_layout.itemAt(i).widget().setParent(None)
                
                # Create new visualization with the correct output directory
                self.visualizer = NetworkVisualizer(output_dir=self.output_trial_directory)
                
                # Add the visualization to the tab
                self.visualization_layout.addWidget(self.visualizer)
                self.output_text.append("Visualization updated with new simulation data.")
                
            except Exception as e:
                self.output_text.append(f"Error loading visualization data: {str(e)}")
        
        self.start_button.setEnabled(True)
        self.stop_button.setEnabled(False)

    def load_kpi_data(self):
        """Load KPI data from the selected trial directory"""
        if not self.output_trial_directory:
            QMessageBox.warning(self, "Warning", "Please select an input trial directory first!")
            return
            
        trial_num = self.trial_spinbox.value()
        round_num = self.kpi_round_spinbox.value()
        
        # Clear existing data
        for table in [self.performance_metrics_table, self.network_statistics_table, self.channel_metrics_table]:
            table.setRowCount(0)
        
        # Construct the file path using the selected output directory
        file_path = os.path.join(self.output_trial_directory, f"performanceResults_{round_num}.txt")
        
        try:
            with open(file_path, 'r') as file:
                lines = file.readlines()
                
                # Performance Metrics Table
                self.performance_metrics_table.setRowCount(2)
                
                # Average Delivery Time
                self.performance_metrics_table.setItem(0, 0, QTableWidgetItem("Average Delivery Time"))
                for i, line in enumerate(lines):
                    if line.startswith("Average delivery time [ms]"):
                        value = float(lines[i+1].strip())
                        self.performance_metrics_table.setItem(0, 1, QTableWidgetItem(f"{value:.3f} ms"))
                        break
                
                # Average Jitter
                self.performance_metrics_table.setItem(1, 0, QTableWidgetItem("Average Jitter"))
                for i, line in enumerate(lines):
                    if line.startswith("Average jitter [ms]"):
                        value = float(lines[i+1].strip())
                        self.performance_metrics_table.setItem(1, 1, QTableWidgetItem(f"{value:.3f} ms"))
                        break
                
                # Channel Metrics Table
                self.channel_metrics_table.setRowCount(2)
                
                # Satellite Throughput
                self.channel_metrics_table.setItem(0, 0, QTableWidgetItem("Satellite Throughput"))
                for i, line in enumerate(lines):
                    if line.startswith("Average satellite aggregated throughput"):
                        value = float(lines[i+1].strip())
                        self.channel_metrics_table.setItem(0, 1, QTableWidgetItem(f"{value:.3f} Mbps"))
                        break
                
                # Terrestrial Throughput
                self.channel_metrics_table.setItem(1, 0, QTableWidgetItem("Terrestrial Throughput"))
                for i, line in enumerate(lines):
                    if line.startswith("Average terrestrial aggregated throughput"):
                        value = float(lines[i+1].strip())
                        self.channel_metrics_table.setItem(1, 1, QTableWidgetItem(f"{value:.3f} Mbps"))
                        break
                
                # Network Statistics Table
                self.network_statistics_table.setRowCount(3)
                
                # Cell Occupancy
                self.network_statistics_table.setItem(0, 0, QTableWidgetItem("Cell Occupancy"))
                for i, line in enumerate(lines):
                    if line.startswith("Cell occupancy per satellite"):
                        values = [int(x) for x in lines[i+1].strip().split()]
                        avg_value = sum(values) / len(values) if values else 0
                        self.network_statistics_table.setItem(0, 1, QTableWidgetItem(f"{avg_value:.0f} UEs"))
                        break
                
                # Satellite Handovers
                self.network_statistics_table.setItem(1, 0, QTableWidgetItem("Satellite Handovers"))
                for i, line in enumerate(lines):
                    if line.startswith("Number of satellite handovers"):
                        value = int(lines[i+1].strip())
                        self.network_statistics_table.setItem(1, 1, QTableWidgetItem(str(value)))
                        break
                
                # Terrestrial Handovers
                self.network_statistics_table.setItem(2, 0, QTableWidgetItem("Terrestrial Handovers"))
                for i, line in enumerate(lines):
                    if line.startswith("Number of terrestrial handovers"):
                        value = int(lines[i+1].strip())
                        self.network_statistics_table.setItem(2, 1, QTableWidgetItem(str(value)))
                        break
                
                # Style the tables
                for table in [self.performance_metrics_table, self.network_statistics_table, self.channel_metrics_table]:
                    for row in range(table.rowCount()):
                        # Make metric names bold
                        item = table.item(row, 0)
                        if item:
                            font = item.font()
                            font.setBold(True)
                            item.setFont(font)
                        # Right-align values
                        value_item = table.item(row, 1)
                        if value_item:
                            value_item.setTextAlignment(Qt.AlignRight | Qt.AlignVCenter)
                    
                    table.resizeColumnsToContents()
                    table.resizeRowsToContents()
                
                QMessageBox.information(self, "Success", "KPI data loaded successfully!")
                
        except FileNotFoundError:
            QMessageBox.warning(self, "Error", f"Performance results file not found: {file_path}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Error loading KPI data: {str(e)}")

    def update_kpi_display(self):
        """Update the KPI display with the latest results"""
        try:
            # Get the current trial number and round
            trial_num = self.trial_spinbox.value()
            round_num = self.kpi_round_spinbox.value()
            
            # Construct the file path with the correct trial directory
            current_dir = os.path.dirname(os.path.abspath(__file__))
            file_path = os.path.join(os.path.dirname(current_dir), "Output", f"trial{trial_num}Output", f"performanceResults_{round_num}.txt")
            
            try:
                with open(file_path, 'r') as file:
                    data = file.readlines()
                    
                # Clear previous results
                for widget in self.kpi_tab.winfo_children():
                    widget.deleteLater()
                    
                # Create header
                header_frame = QGroupBox("KPI Results")
                header_layout = QVBoxLayout(header_frame)
                header_layout.addWidget(QLabel("KPI Results", font=QFont("Helvetica", 12, QFont.Bold)))
                
                # Create results display
                results_frame = QGroupBox("Results")
                results_layout = QVBoxLayout(results_frame)
                
                # Create text widget with scrollbar
                text_widget = QTextEdit()
                text_widget.setReadOnly(True)
                text_widget.setPlainText("".join(data))
                results_layout.addWidget(text_widget)
                
                # Add frames to layout
                self.kpi_tab.layout().addWidget(header_frame)
                self.kpi_tab.layout().addWidget(results_frame)
                
            except FileNotFoundError:
                # Create error message
                error_frame = QGroupBox("Error")
                error_layout = QVBoxLayout(error_frame)
                error_layout.addWidget(QLabel(f"Results file not found: {file_path}"))
                error_layout.addWidget(QLabel("Please ensure the simulation has been run and the results file exists."))
                
                # Add error frame to layout
                self.kpi_tab.layout().addWidget(error_frame)
                
        except Exception as e:
            print(f"Error updating KPI display: {str(e)}")
            # Create error message
            error_frame = QGroupBox("Error")
            error_layout = QVBoxLayout(error_frame)
            error_layout.addWidget(QLabel(f"Error: {str(e)}"))
            error_layout.addWidget(QLabel("Please check the console for more details."))
            
            # Add error frame to layout
            self.kpi_tab.layout().addWidget(error_frame)

def main():
    app = QApplication(sys.argv)
    window = SimulationGUI()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
