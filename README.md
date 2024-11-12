## 🚀 Apollo-inspired LEM Simulation
This simulation models the descent, landing, and sample retrieval mission of a lunar lander (LEM) based on the Apollo missions. The project highlights real-time control, navigation through an asteroid belt, and interaction with a planet’s gravitational environment.

### 🌌 Project Overview
In this scaled-down environment, a Lander detaches from the command module, navigates an asteroid field, lands on a planetary surface, retrieves a sample, and returns. The planet's characteristics are based on approximate astronomical values to provide realistic gravitational effects for the LEM, modeled as a simplified Apollo descent vehicle.

### 🔬 Physics Models
**Vertical Descent**: Controlled by a primary thruster producing a constant force, managing descent speed against gravity.

**Horizontal Movement**: Achieved through minor angular adjustments (max ±10°) to prevent excess vertical speed. Side thrusters generate torque to angle the LEM, introducing horizontal force.

**Asteroid Simulation**: Asteroids orbit at calculated velocities based on gravitational equilibrium, creating an obstacle for the LEM.

The simulation is based on kinematic equations discretized for real-time application, with primary variables like LEM mass, fuel consumption, and planetary mass contributing to the dynamics. 

**Thruster and Torque Control**: Manages descent speed and horizontal velocity using discrete-time transfer functions.

**Collision Avoidance**: Asteroid positions and movements are periodically recalculated to maintain them within the visible screen area, creating a dynamic challenge for the LEM.

### 🧩 Key Components and Tasks
**LEM Task**: Updates position, velocity, fuel, and verifies landing conditions. This task runs every 20ms, ensuring smooth movement visible at 20 FPS.

**Asteroid Task**: Manages multiple asteroids with checks to ensure they remain on-screen.

**Graphics Task**: Converts physical coordinates to pixel positions at a 3-pixel per meter ratio, rendering the environment.

**Keyboard Task**: Processes user inputs (every 200ms) for LEM mass adjustments, asteroid counts, and thruster control.

**Autopilot Task**: Activates the thrusters and adjusts speed based on asteroid proximity.

Global variables (e.g., LEM mass and asteroid data) are accessed with mutex locks to ensure safe concurrent modifications across tasks. Each task adheres to a fixed period for real-time consistency, with critical tasks prioritized to prevent delays.

---
### 🛠️ Installation and Execution
To compile, navigate to the project directory and run:
`gcc -s main.c $(allegro-config --libs) -pthread -o main -lm`
Run the Simulation (Admin privileges required):
`sudo ./main`
