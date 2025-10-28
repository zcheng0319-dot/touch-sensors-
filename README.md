# Touch-Sensitive Lighting System
This project features an interactive light system built with an **Arduino MKR1010**, designed to change its color in response to physical touch.  
The goal was to explore how simple digital interaction can create an emotional response through light.

## Concept
The idea was inspired by Duncan’s demo of a simple box structure.  
I wanted to make something minimal too — a small box that reacts to touch — but with an added **breathing light** effect.  
The inspiration came from the idea of *“emotional light”*, where light behaves more like a living object than a mechanical switch.  
When touched, the light “breathes” softly through color transitions, creating a calm and human-like interaction.

## Development
The system uses a **touch sensor** to detect contact and sends data through the **MQTT broker**, which then controls the RGB LEDs.  
I adjusted the code to include a smooth fade-to-color function, allowing the light to transition gradually rather than change instantly.  
This small modification makes the interaction feel more natural and expressive.

## Reflection
Through this project, I learned how to combine **sensor input**, **MQTT communication**, and **visual feedback** into a connected system.  
It also helped me understand how subtle interaction design can change the emotional tone of a device, making technology feel more human.
