import sys
import os
from PyQt5.QtWidgets import QApplication

# Import the NetworkVisualizer directly
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "anchor-simulator/visualisation"))
from network_visualizer import NetworkVisualizer

def main():
    app = QApplication(sys.argv)
    
    # Create the visualizer
    visualizer = NetworkVisualizer()
    
    # Show the window
    visualizer.show()
    
    # Run the application
    sys.exit(app.exec_())

if __name__ == "__main__":
    main() 