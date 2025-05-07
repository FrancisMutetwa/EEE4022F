import os
import numpy as np
import matplotlib
matplotlib.use('Agg')  # Use Agg backend for better performance
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from PyQt5.QtWidgets import QWidget, QVBoxLayout
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
import pandas as pd

class NetworkVisualizer(QWidget):
    def __init__(self, output_dir=None):
        super().__init__()
        
        # Set up the output directory
        if output_dir is None:
            script_dir = os.path.dirname(os.path.abspath(__file__))
            anchor_simulator_dir = os.path.dirname(script_dir)
            output_dir = os.path.join(anchor_simulator_dir, "Output", "trial1Output")
        self.output_dir = output_dir
        
        # Earth parameters
        self.earth_radius = 6371  # Earth radius in kilometers
        
        # Create the figure and 3D axes
        self.fig = plt.figure(figsize=(10, 8))
        self.ax = self.fig.add_subplot(111, projection='3d')
        
        # Create the canvas
        self.canvas = FigureCanvas(self.fig)
        
        # Add navigation toolbar
        self.toolbar = NavigationToolbar(self.canvas, self)
        
        # Set up the layout
        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)
        
        # Load and plot the data
        self.load_and_plot_data()
    
    def draw_earth(self):
        """Draw a wireframe sphere representing Earth"""
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
    
    def load_and_plot_data(self):
        """Load node positions and plot them"""
        try:
            # Clear previous plot
            self.ax.clear()
            
            # Load node positions
            df = pd.read_csv(os.path.join(self.output_dir, "node_positions.csv"))
            
            # Plot Earth
            self.draw_earth()
            
            # Plot nodes
            satellite_nodes = []
            terrestrial_nodes = []
            ue_nodes = []
            
            for node_id in df['NodeId'].unique():
                node_data = df[df['NodeId'] == node_id]
                x = node_data['X'].iloc[0]
                y = node_data['Y'].iloc[0]
                z = node_data['Z'].iloc[0]
                
                if node_id < 10:  # Satellite gNBs (0-9)
                    satellite_nodes.append((node_id, x, y, z))
                elif node_id < 20:  # Terrestrial gNBs (10-19)
                    terrestrial_nodes.append((node_id, x, y, z))
                elif node_id >= 1000:  # UEs (1000+)
                    ue_nodes.append((node_id, x, y, z))
            
            # Plot satellite gNBs
            if satellite_nodes:
                sat_x, sat_y, sat_z = zip(*[(x, y, z) for _, x, y, z in satellite_nodes])
                sat_scatter = self.ax.scatter(sat_x, sat_y, sat_z, c='red', marker='^', s=100, label='Satellite gNB')
                for node_id, x, y, z in satellite_nodes:
                    self.ax.text(x, y, z, f'Sat {node_id}', color='red', fontsize=8, 
                               bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))
            
            # Plot terrestrial gNBs
            if terrestrial_nodes:
                ter_x, ter_y, ter_z = zip(*[(x, y, z) for _, x, y, z in terrestrial_nodes])
                ter_scatter = self.ax.scatter(ter_x, ter_y, ter_z, c='green', marker='s', s=100, label='Terrestrial gNB')
                for node_id, x, y, z in terrestrial_nodes:
                    self.ax.text(x, y, z, f'Ter {node_id-10}', color='green', fontsize=8,
                               bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))
            
            # Plot UEs
            if ue_nodes:
                ue_x, ue_y, ue_z = zip(*[(x, y, z) for _, x, y, z in ue_nodes])
                ue_scatter = self.ax.scatter(ue_x, ue_y, ue_z, c='blue', marker='o', s=150, label='UE')
                for node_id, x, y, z in ue_nodes:
                    self.ax.text(x, y, z, f'UE {node_id-1000}', color='blue', fontsize=8,
                               bbox=dict(facecolor='white', alpha=0.7, edgecolor='none'))
            
            # Set labels and title
            self.ax.set_xlabel('X (km)')
            self.ax.set_ylabel('Y (km)')
            self.ax.set_zlabel('Z (km)')
            self.ax.set_title('Network Configuration')
            
            # Add legend
            handles = []
            if satellite_nodes:
                handles.append(sat_scatter)
            if terrestrial_nodes:
                handles.append(ter_scatter)
            if ue_nodes:
                handles.append(ue_scatter)
            self.ax.legend(handles=handles)
            
            # Set equal aspect ratio
            self.ax.set_box_aspect([1, 1, 1])
            
            # Update the canvas
            self.canvas.draw()
            
        except Exception as e:
            print(f"Error in visualization: {str(e)}")
            import traceback
            traceback.print_exc() 