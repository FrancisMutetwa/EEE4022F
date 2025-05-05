def load_data(self):
        """Load simulation data from CSV file."""
        try:
            # Initialize data structures
            self.time_values = set()
            self.node_positions = {}  # {time: {nodeId: (x,y,z,type)}}
            self.connections = {}     # {time: [(source_id, target_id)]}
            self.udp_packets = {}     # {time: [(source_id, target_id, size)]}
            self.satellite_movements = {}  # {time: [(sat_id, old_pos, new_pos)]}
            
            # Track unique nodes
            unique_nodes = {
                'satellite_gNB': set(),
                'terrestrial_gNB': set(),
                'UE': set()
            }

            with open(self.data_file, 'r') as f:
                # Skip header
                next(f)
                
                for line in f:
                    line = line.strip()
                    if not line:
                        print("Warning: Skipping empty line")
                        continue
                        
                    try:
                        time, event_type, data = line.split(',', 2)
                        time = float(time)
                        data = json.loads(data)
                        
                        self.time_values.add(time)
                        
                        if event_type == 'node_position':
                            if time not in self.node_positions:
                                self.node_positions[time] = {}
                                
                            node_id = data['nodeId']
                            node_type = data['nodeType']
                            pos = (data['x'], data['y'], data['z'])
                            
                            self.node_positions[time][node_id] = (*pos, node_type)
                            unique_nodes[node_type].add(node_id)
                            
                        elif event_type == 'connection':
                            if time not in self.connections:
                                self.connections[time] = []
                            self.connections[time].append((data['ueId'], data['cellId']))
                            
                        elif event_type == 'udp_packet':
                            if time not in self.udp_packets:
                                self.udp_packets[time] = []
                            self.udp_packets[time].append((data['sourceId'], data['targetId'], data.get('size', 0)))
                            
                        elif event_type == 'satellite_movement':
                            if time not in self.satellite_movements:
                                self.satellite_movements[time] = []
                            self.satellite_movements[time].append((data['satelliteId'], data['oldPosition'], data['newPosition']))
                            
                    except (ValueError, json.JSONDecodeError) as e:
                        print(f"Error: {str(e)}")
                        print(f"Warning: Skipping malformed line: {line}")
                        continue

            self.time_values = sorted(list(self.time_values))
            if not self.time_values:
                raise ValueError("No valid time values found in data file")

            print("\nDebug Information:")
            print(f"Total unique nodes loaded: {sum(len(nodes) for nodes in unique_nodes.values())}")
            print(f"Satellite gNBs: {len(unique_nodes['satellite_gNB'])}")
            print(f"Terrestrial gNBs: {len(unique_nodes['terrestrial_gNB'])}")
            print(f"UEs: {len(unique_nodes['UE'])}")
            print(f"Time steps: {len(self.time_values)}")
            print(f"Time range: {min(self.time_values)} to {max(self.time_values)}")
            print("Data loaded successfully with", len(self.time_values), "time steps\n")

        except FileNotFoundError:
            print(f"Error: Could not find data file at {self.data_file}")
            raise
        except Exception as e:
            print(f"Error loading data: {str(e)}")
            raise 

def plot_nodes(self, time):
        """Plot nodes at a specific time."""
        if time not in self.node_positions:
            print(f"Warning: No node positions found for time {time}")
            return

        # Clear previous plots
        for collection in self.ax.collections:
            collection.remove()
        for text in self.ax.texts:
            text.remove()

        # Track node counts for this time step
        node_counts = {
            'satellite_gNB': 0,
            'terrestrial_gNB': 0,
            'UE': 0
        }

        # Plot nodes
        for node_id, (x, y, z, node_type) in self.node_positions[time].items():
            # Convert coordinates from meters to kilometers
            x, y, z = x/1000, y/1000, z/1000
            
            if node_type == 'satellite_gNB':
                color = 'red'
                marker = '^'
                size = 100
            elif node_type == 'terrestrial_gNB':
                color = 'blue'
                marker = 's'
                size = 100
            else:  # UE
                color = 'green'
                marker = 'o'
                size = 50

            self.ax.scatter(x, y, z, c=color, marker=marker, s=size)
            node_counts[node_type] += 1

            # Add node label
            self.ax.text(x, y, z, f'{node_type}_{node_id}', fontsize=8)

        # Plot connections at this time step
        if time in self.connections:
            for source_id, target_id in self.connections[time]:
                if source_id in self.node_positions[time] and target_id in self.node_positions[time]:
                    source_pos = self.node_positions[time][source_id][:3]
                    target_pos = self.node_positions[time][target_id][:3]
                    # Convert to kilometers
                    source_pos = [p/1000 for p in source_pos]
                    target_pos = [p/1000 for p in target_pos]
                    self.ax.plot([source_pos[0], target_pos[0]], 
                               [source_pos[1], target_pos[1]], 
                               [source_pos[2], target_pos[2]], 
                               'k--', alpha=0.3)

        # Update plot title with current time and node counts
        print(f"\nPlotting at time {time}:")
        print(f"Satellite gNBs: {node_counts['satellite_gNB']}")
        print(f"Terrestrial gNBs: {node_counts['terrestrial_gNB']}")
        print(f"UEs: {node_counts['UE']}")
        print(f"Total nodes: {sum(node_counts.values())}")
        print(f"Connections: {len(self.connections.get(time, []))}")
        print(f"UDP Packets: {len(self.udp_packets.get(time, []))}")
        print(f"Satellite Movements: {len(self.satellite_movements.get(time, []))}")

        self.ax.set_title(f'Network State at t={time:.1f}s\n'
                         f'Satellites: {node_counts["satellite_gNB"]}, '
                         f'Terrestrial: {node_counts["terrestrial_gNB"]}, '
                         f'UEs: {node_counts["UE"]}')

        # Draw Earth's surface
        if not hasattr(self, 'earth_surface'):
            # Create Earth's surface as a wireframe sphere
            radius = 6371  # Earth's radius in kilometers
            u = np.linspace(0, 2 * np.pi, 100)
            v = np.linspace(0, np.pi, 50)
            x = radius * np.outer(np.cos(u), np.sin(v))
            y = radius * np.outer(np.sin(u), np.sin(v))
            z = radius * np.outer(np.ones(np.size(u)), np.cos(v))
            self.earth_surface = self.ax.plot_wireframe(x, y, z, color='gray', alpha=0.1)

        self.canvas.draw() 