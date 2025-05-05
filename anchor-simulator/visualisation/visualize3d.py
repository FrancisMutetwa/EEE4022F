import os
import sys
import json
import numpy as np
import matplotlib
# Force Agg backend before importing pyplot
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animation
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton, QSlider, QLabel, QMessageBox
from PyQt5.QtCore import Qt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas

class SimulationVisualizer(QWidget):
    def __init__(self, output_dir=None):
        try:
            super().__init__()
            # If output_dir is not provided, try to find it relative to the script location
            if output_dir is None:
                script_dir = os.path.dirname(os.path.abspath(__file__))
                # Go up one directory from visualisation to anchor-simulator
                anchor_simulator_dir = os.path.dirname(script_dir)
                output_dir = os.path.join(anchor_simulator_dir, "Output", "trial2Output")
            self.output_dir = output_dir
            self.current_time = 0
            self.earth_radius = 6371  # Earth radius in kilometers
            
            print("Creating figure...")
            # Create the main figure and 3D axes
            self.fig = plt.figure(figsize=(10, 8))
            self.ax = self.fig.add_subplot(111, projection='3d')
            print("Creating canvas...")
            self.canvas = FigureCanvas(self.fig)
            
            # Create layout
            layout = QVBoxLayout()
            layout.addWidget(self.canvas)
            
            # Add time slider
            self.time_slider = QSlider(Qt.Horizontal)
            self.time_slider.setMinimum(0)
            self.time_slider.setMaximum(100)
            self.time_slider.valueChanged.connect(self.update_time)
            layout.addWidget(self.time_slider)
            
            # Add time label
            self.time_label = QLabel("Time: 0.0s")
            layout.addWidget(self.time_label)
            
            # Add control buttons
            button_layout = QVBoxLayout()
            self.play_button = QPushButton("Play")
            self.play_button.clicked.connect(self.toggle_animation)
            button_layout.addWidget(self.play_button)
            
            self.reset_button = QPushButton("Reset")
            self.reset_button.clicked.connect(self.reset_animation)
            button_layout.addWidget(self.reset_button)
            
            layout.addLayout(button_layout)
            self.setLayout(layout)
            
            # Animation
            self.animation = None
            self.is_playing = False
            
            print("Loading data...")
            # Load data
            self.load_data()
            
            # Update plot
            if hasattr(self, 'time_values') and self.time_values:
                print("Updating initial plot...")
                self.update_plot(self.time_values[0])
            else:
                print("No data loaded")
                QMessageBox.warning(self, "Warning", "No data was loaded.")
        except Exception as e:
            print(f"Error in initialization: {str(e)}")
            import traceback
            traceback.print_exc()
    
    def load_data(self):
        """Load simulation data from CSV file"""
        try:
            # Create output directory if it doesn't exist
            os.makedirs(self.output_dir, exist_ok=True)
            
            file_path = os.path.join(self.output_dir, "simulation_events.csv")
            print(f"Loading data from {file_path}")
            
            if not os.path.exists(file_path):
                print(f"Error: File not found at {file_path}")
                print("Current working directory:", os.getcwd())
                print("Output directory:", self.output_dir)
                print("Available files in output directory:", os.listdir(self.output_dir) if os.path.exists(self.output_dir) else "Directory not found")
                
                # Initialize empty data structures
                self.node_positions = {}
                self.connections = {}
                self.time_values = []
                return
            
            # Initialize data structures
            self.node_positions = {}  # {time: {node_id: (x, y, z, node_type)}}
            self.connections = {}     # {time: [(node1, node2)]}
            self.time_values = set()
            
            # Debug counters
            node_count = 0
            satellite_count = 0
            terrestrial_count = 0
            ue_count = 0
            
            # Read CSV file line by line
            with open(file_path, 'r') as f:
                # Skip header
                next(f)
                
                for line in f:
                    try:
                        # Split line into components
                        parts = line.strip().split(',', 2)
                        if len(parts) != 3:
                            print(f"Warning: Skipping malformed line: {line.strip()}")
                            continue
                            
                        time = float(parts[0])
                        event_type = parts[1]
                        data_str = parts[2].strip('"')  # Remove outer quotes
                        data_str = data_str.replace('""', '"')  # Replace escaped double quotes with single quotes
                        
                        # Skip if data is None or empty
                        if data_str == 'None' or not data_str:
                            continue
                            
                        # Parse JSON data
                        data = json.loads(data_str)
                        
                        # Initialize time entries if needed
                        if time not in self.node_positions:
                            self.node_positions[time] = {}
                        if time not in self.connections:
                            self.connections[time] = []
                        
                        if event_type == "node_position":
                            node_id = data["nodeId"]
                            node_type = data["nodeType"]
                            x = float(data["x"])
                            y = float(data["y"])
                            z = float(data["z"])
                            
                            self.time_values.add(time)
                            self.node_positions[time][node_id] = (x, y, z, node_type)
                            
                            # Count node types
                            node_count += 1
                            if node_type == "satellite_gNB":
                                satellite_count += 1
                            elif node_type == "terrestrial_gNB":
                                terrestrial_count += 1
                            elif node_type == "UE":
                                ue_count += 1
                        
                        elif event_type == "connection":
                            ue_id = data["ueId"]
                            cell_id = data["cellId"]
                            self.connections[time].append((ue_id, cell_id))
                    
                    except (ValueError, json.JSONDecodeError) as e:
                        print(f"Error processing line: {line.strip()}: {str(e)}")
                        continue
            
            # Print debug information
            print(f"\nDebug Information:")
            print(f"Total nodes loaded: {node_count}")
            print(f"Satellite gNBs: {satellite_count}")
            print(f"Terrestrial gNBs: {terrestrial_count}")
            print(f"UEs: {ue_count}")
            print(f"Time steps: {len(self.time_values)}")
            
            # Convert time_values to sorted list
            self.time_values = sorted(list(self.time_values))
            
            if not self.time_values:
                print("No valid time values found in the data")
                return
            
            print(f"Time range: {self.time_values[0]} to {self.time_values[-1]}")
            print(f"Data loaded successfully with {len(self.time_values)} time steps")
            
            # Update slider range
            self.time_slider.setMaximum(len(self.time_values) - 1)
            
        except Exception as e:
            print(f"Error loading data: {str(e)}")
            import traceback
            traceback.print_exc()
            # Initialize empty data structures
            self.node_positions = {}
            self.connections = {}
            self.time_values = []
            return
    
    def draw_earth_surface(self):
        """Draw a wireframe sphere representing Earth's surface"""
        # Create a sphere
        phi = np.linspace(0, np.pi, 20)
        theta = np.linspace(0, 2 * np.pi, 40)
        phi, theta = np.meshgrid(phi, theta)

        x = self.earth_radius * np.sin(phi) * np.cos(theta)
        y = self.earth_radius * np.sin(phi) * np.sin(theta)
        z = self.earth_radius * np.cos(phi)

        # Plot the wireframe sphere
        self.ax.plot_wireframe(x, y, z, color='gray', alpha=0.3)
        
        # Add a semi-transparent surface
        self.ax.plot_surface(x, y, z, color='blue', alpha=0.1)
    
    def update_plot(self, time):
        """Update the plot for the given time"""
        # Clear previous plot
        self.ax.clear()
        
        # Draw Earth surface
        self.draw_earth_surface()
        
        # Get node positions for current time
        if time in self.node_positions:
            current_positions = self.node_positions[time]
            
            # Debug counters for current time step
            current_satellite_count = 0
            current_terrestrial_count = 0
            current_ue_count = 0
            
            # Create a mapping of node types to their unique IDs
            node_type_map = {
                "satellite_gNB": set(),
                "terrestrial_gNB": set(),
                "UE": set()
            }
            
            # First pass: collect all node IDs by type
            for node_id, (x, y, z, node_type) in current_positions.items():
                if node_type in node_type_map:
                    node_type_map[node_type].add(node_id)
            
            # Plot nodes
            for node_id, (x, y, z, node_type) in current_positions.items():
                # Scale coordinates to match Earth's radius
                # The original coordinates are in meters, so we divide by 1000 to get km
                x_scaled = x / 1000
                y_scaled = y / 1000
                z_scaled = z / 1000
                
                # Create a unique label for each node
                node_index = sorted(list(node_type_map[node_type])).index(node_id)
                label = f"{node_type} {node_index}"
                
                if node_type == "satellite_gNB":
                    self.ax.scatter(x_scaled, y_scaled, z_scaled, c='red', s=200, label=label, marker='^')
                    current_satellite_count += 1
                elif node_type == "terrestrial_gNB":
                    self.ax.scatter(x_scaled, y_scaled, z_scaled, c='blue', s=200, label=label, marker='s')
                    current_terrestrial_count += 1
                elif node_type == "UE":
                    self.ax.scatter(x_scaled, y_scaled, z_scaled, c='green', s=100, label=label, marker='o')
                    current_ue_count += 1
                else:
                    print(f"Warning: Unknown node type '{node_type}' for node {node_id}")
            
            # Print debug information for current time step
            print(f"\nPlotting at time {time}:")
            print(f"Satellite gNBs: {current_satellite_count}")
            print(f"Terrestrial gNBs: {current_terrestrial_count}")
            print(f"UEs: {current_ue_count}")
            print(f"Total nodes: {len(current_positions)}")
        
        # Plot connections
        if time in self.connections:
            connection_count = len(self.connections[time])
            print(f"Connections: {connection_count}")
            for node1, node2 in self.connections[time]:
                if node1 in current_positions and node2 in current_positions:
                    x1, y1, z1, _ = current_positions[node1]
                    x2, y2, z2, _ = current_positions[node2]
                    # Scale connection coordinates
                    x1_scaled = x1 / 1000
                    y1_scaled = y1 / 1000
                    z1_scaled = z1 / 1000
                    x2_scaled = x2 / 1000
                    y2_scaled = y2 / 1000
                    z2_scaled = z2 / 1000
                    self.ax.plot([x1_scaled, x2_scaled], [y1_scaled, y2_scaled], [z1_scaled, z2_scaled], 'k--', alpha=0.5)
        
        # Set plot properties
        self.ax.set_xlabel('X (km)')
        self.ax.set_ylabel('Y (km)')
        self.ax.set_zlabel('Z (km)')
        self.ax.set_title(f'Network Visualization (Time: {time:.1f}s)')
        
        # Set equal aspect ratio
        self.ax.set_box_aspect([1, 1, 1])
        
        # Set axis limits to show the Earth and nodes
        # Use a larger limit to accommodate satellite positions
        limit = self.earth_radius * 2
        self.ax.set_xlim(-limit, limit)
        self.ax.set_ylim(-limit, limit)
        self.ax.set_zlim(-limit, limit)
        
        # Add legend
        handles, labels = self.ax.get_legend_handles_labels()
        by_label = dict(zip(labels, handles))
        self.ax.legend(by_label.values(), by_label.keys())
        
        # Redraw the plot
        self.canvas.draw()
    
    def update_time(self, slider_value):
        """Update visualization based on slider value"""
        if self.time_values:
            time = self.time_values[slider_value]
            self.current_time = time
            self.time_label.setText(f"Time: {time:.1f}s")
            self.update_plot(time)
    
    def animate(self, frame):
        """Animation update function"""
        if self.is_playing and frame < len(self.time_values):
            self.time_slider.setValue(frame)
            return self.ax,
    
    def toggle_animation(self):
        """Toggle animation play/pause"""
        if not hasattr(self, 'time_values') or not self.time_values:
            print("No data available for animation")
            return
            
        self.is_playing = not self.is_playing
        if self.is_playing:
            self.play_button.setText("Pause")
            if not self.animation:
                self.animation = animation.FuncAnimation(
                    self.fig, self.animate,
                    frames=len(self.time_values),
                    interval=100,
                    blit=True
                )
        else:
            self.play_button.setText("Play")
    
    def reset_animation(self):
        """Reset animation to start"""
        self.time_slider.setValue(0)
        self.is_playing = False
        self.play_button.setText("Play")
        if self.animation:
            self.animation.event_source.stop()
            self.animation = None

def plot_network(output_dir, time_step=0.0):
    """Plot the network at a specific time step and save to file"""
    try:
        # Create figure
        fig = plt.figure(figsize=(12, 10))
        ax = fig.add_subplot(111, projection='3d')
        
        # Load data from CSV
        file_path = os.path.join(output_dir, "simulation_events.csv")
        print(f"Loading data from {file_path}")
        
        if not os.path.exists(file_path):
            print(f"Error: File not found at {file_path}")
            return
        
        # Initialize data structures
        node_positions = {}  # {node_id: (x, y, z, node_type)}
        connections = []     # [(node1, node2)]
        
        # Read CSV file
        with open(file_path, 'r') as f:
            next(f)  # Skip header
            for line in f:
                try:
                    parts = line.strip().split(',', 2)
                    if len(parts) != 3:
                        continue
                    
                    time = float(parts[0])
                    if time != time_step:
                        continue
                        
                    event_type = parts[1]
                    data_str = parts[2].strip('"').replace('""', '"')
                    
                    if data_str == 'None' or not data_str:
                        continue
                    
                    data = json.loads(data_str)
                    
                    if event_type == "node_position":
                        node_id = data["nodeId"]
                        node_type = data["nodeType"]
                        x = float(data["x"]) / 1000  # Convert to km
                        y = float(data["y"]) / 1000
                        z = float(data["z"]) / 1000
                        node_positions[node_id] = (x, y, z, node_type)
                    
                    elif event_type == "connection":
                        ue_id = data["ueId"]
                        cell_id = data["cellId"]
                        connections.append((ue_id, cell_id))
                
                except (ValueError, json.JSONDecodeError) as e:
                    print(f"Error processing line: {line.strip()}: {str(e)}")
                    continue
        
        # Draw Earth
        earth_radius = 6371  # km
        phi = np.linspace(0, np.pi, 20)
        theta = np.linspace(0, 2 * np.pi, 40)
        phi, theta = np.meshgrid(phi, theta)

        x = earth_radius * np.sin(phi) * np.cos(theta)
        y = earth_radius * np.sin(phi) * np.sin(theta)
        z = earth_radius * np.cos(phi)

        ax.plot_wireframe(x, y, z, color='gray', alpha=0.3)
        ax.plot_surface(x, y, z, color='blue', alpha=0.1)
        
        # Plot nodes
        satellite_count = 0
        terrestrial_count = 0
        ue_count = 0
        
        for node_id, (x, y, z, node_type) in node_positions.items():
            if node_type == "satellite_gNB":
                ax.scatter(x, y, z, c='red', s=200, label=f'Satellite gNB {satellite_count}', marker='^')
                satellite_count += 1
            elif node_type == "terrestrial_gNB":
                ax.scatter(x, y, z, c='blue', s=200, label=f'Terrestrial gNB {terrestrial_count}', marker='s')
                terrestrial_count += 1
            elif node_type == "UE":
                ax.scatter(x, y, z, c='green', s=100, label=f'UE {ue_count}', marker='o')
                ue_count += 1
        
        # Plot connections
        for node1, node2 in connections:
            if node1 in node_positions and node2 in node_positions:
                x1, y1, z1, _ = node_positions[node1]
                x2, y2, z2, _ = node_positions[node2]
                ax.plot([x1, x2], [y1, y2], [z1, z2], 'k--', alpha=0.5)
        
        # Set plot properties
        ax.set_xlabel('X (km)')
        ax.set_ylabel('Y (km)')
        ax.set_zlabel('Z (km)')
        ax.set_title(f'Network Visualization (Time: {time_step:.1f}s)')
        
        # Set equal aspect ratio
        ax.set_box_aspect([1, 1, 1])
        
        # Set axis limits
        limit = earth_radius * 2
        ax.set_xlim(-limit, limit)
        ax.set_ylim(-limit, limit)
        ax.set_zlim(-limit, limit)
        
        # Add legend
        handles, labels = ax.get_legend_handles_labels()
        by_label = dict(zip(labels, handles))
        ax.legend(by_label.values(), by_label.keys())
        
        # Save plot
        output_file = os.path.join(output_dir, f"network_visualization_t{time_step:.1f}.png")
        print(f"Saving plot to {output_file}")
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        plt.close()
        
        print(f"\nVisualization statistics at time {time_step}:")
        print(f"Satellite gNBs: {satellite_count}")
        print(f"Terrestrial gNBs: {terrestrial_count}")
        print(f"UEs: {ue_count}")
        print(f"Total nodes: {len(node_positions)}")
        print(f"Connections: {len(connections)}")
        
    except Exception as e:
        print(f"Error creating visualization: {str(e)}")
        import traceback
        traceback.print_exc()

def main():
    try:
        # Enable high DPI support
        if hasattr(Qt, 'AA_EnableHighDpiScaling'):
            QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
        if hasattr(Qt, 'AA_UseHighDpiPixmaps'):
            QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)
        
        app = QApplication(sys.argv)
        print("Qt application initialized")
        
        # Create and show the main window
        window = QMainWindow()
        print("Main window created")
        
        try:
            # Create the visualizer
            visualizer = SimulationVisualizer()
            print("Visualizer created")
            
            # Set up the window
            window.setCentralWidget(visualizer)
            window.setWindowTitle("Network Visualization")
            window.resize(1200, 800)
            print("Window configured")
            
            # Show the window
            window.show()
            print("Window shown")
            
            # Start the event loop
            print("Starting Qt event loop")
            return app.exec_()
        except Exception as e:
            print(f"Error in visualization setup: {str(e)}")
            import traceback
            traceback.print_exc()
            QMessageBox.critical(window, "Error", f"Failed to initialize visualization: {str(e)}")
            return 1
            
    except Exception as e:
        print(f"Error in application setup: {str(e)}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == "__main__":
    print("Starting visualization script")
    sys.exit(main()) 