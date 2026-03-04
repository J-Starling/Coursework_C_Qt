Locomotive Fuel Efficiency Analyzer (Diesel vs LNG)

An interactive Qt-based C++ application for comparative economic and environmental analysis of diesel fuel versus liquefied natural gas (LNG) for railway locomotives.

📋 Overview

This application provides a comprehensive tool for analyzing the feasibility of converting railway rolling stock from traditional diesel fuel to LNG. It combines mathematical modeling with interactive visualization to help users understand the economic and environmental impacts of fuel switching.

Key Features:
```
Interactive GUI with dialog-based data input
Real-time animation of locomotive movement and fuel consumption
Economic calculations including annual costs, savings, and payback periods
Environmental impact assessment (NOx, CO₂, particulate matter reduction)
Multiple report formats (TXT and PDF with scene screenshots)
Keyboard and mouse controls for intuitive interaction
Context menus for quick actions
```

🧮 Calculation Engine

The application performs calculations based on the following formulas:

Annual Energy Requirement:
```
Energy_annual = Power × (Annual_mileage / Speed) × 3.6
```
Fuel Consumption (tons):
```
Diesel_consumption = Energy_annual / (Diesel_energy_density × 1000)
LNG_consumption = Energy_annual / (LNG_energy_density × 1000)
```
Annual Fuel Costs:
```
Diesel_cost = Diesel_consumption × Diesel_price_per_ton
LNG_cost = LNG_consumption × LNG_price_per_ton
```
Economic Effect:
```
Annual_savings = Diesel_cost - LNG_cost
Payback_period = Modernization_cost / Annual_savings
```

🎮 Usage Guide

Initial Setup

Name Input: Enter your name (Cyrillic characters, spaces, and hyphens allowed)
Mode Selection: Choose initial fuel type (Diesel or LNG)
Parameter Input: Enter operational and economic parameters:
```
Annual mileage (km)

Average speed (km/h)

Engine power (kW)
Fuel prices (RUB/ton)
Modernization cost (RUB)
```

Main Interface
Left Panel: Animated scene with:
```
Moving locomotive (color-coded: blue for diesel, green for LNG)
Fuel tank with real-time level indicator
Railway track, trees, power lines, station building
Clouds and environmental elements
```
Right Panel: Calculation results display

Controls
Key	Actions:
```
A	Switch to Diesel mode
D	Switch to LNG mode
Space	Start/Pause animation
R	Reset animation
I	Show help
Ctrl+S	Save TXT report
Ctrl+Shift+S	Save PDF report
Ctrl+F	Refuel
```
Mouse Interactions:
```
Right-click on locomotive or fuel tank: Opens context menu with options:
Refuel
Show technical data
Start/Pause animation
Reset animation
```

📊 Reports

The application generates two types of reports:
TXT Report:
```
Plain text format
All input parameters
Calculation results
Technical specifications
Environmental impact data
```
PDF Report:
```
Professional formatting with CSS styling
All data from TXT report
Current scene screenshot
A4 page format
High-resolution output
```

🏗️ Architecture

Class Structure:
```
MainWindow: Central application class managing UI, scene, and calculations
NameDialog: User name input with validation
ModeDialog: Fuel type selection
ParametersDialog: Numerical parameter input
CalculationData: Data structure for all input/output values
```

Key Components:
```
QGraphicsScene: 1000×600 pixel scene for visualization
QTimer: Animation timers for movement and fuel consumption
QPrinter: PDF generation with scene screenshots
QRegularExpressionValidator: Input validation
```

🎨 Visualization Features:
```
Gradient sky and ground backgrounds
Detailed railway track with ballast, rails, and ties
Realistic fuel tank with level indicator
Animated locomotive movement
Environmental elements (clouds, trees, power lines, station)
Dynamic color coding for fuel types
Real-time fuel level updates
```

🔧 Technical Constants:
```
Parameter            Diesel	          LNG
Energy Density	   42.7 MJ/kg	      49.0 MJ/kg
Density	            850 kg/m³	      420 kg/m³

NOx Reduction	-	80-90%
CO₂ Reduction	-	25%
Particulate Reduction	-	90-95%
```
Note: This application was developed as a course project for comparative analysis of railway locomotive fuel efficiency. All calculations are based on published technical data and should be used for educational purposes.
