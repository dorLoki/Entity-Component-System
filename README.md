# 1. ECS Implementation - Multi-Version Overview

## 1.1. What is an ECS?

Entity-Component-System (ECS) is a software architectural pattern commonly used in game development and simulations. It separates data (components) from behavior (systems) and entities are simply unique IDs that aggregate components. This design enables cache-friendly data layouts, easy parallelism, and flexible composition compared to traditional object-oriented approaches.

---

## 1.2. Versions Overview

This repository contains **5 progressively improved ECS implementations**, each demonstrating different design decisions and performance characteristics.

---

## 1.3. Benchmark
| Version | ECS time (ms) | Factor (OOP / ECS) |
|---------|---------------|---------------------|
| v1      | ~1000         | x1.45               |
| v2      | ~1000         | x1.45               |
| v3      | ~580          | x2.50               |
| v4      | ~580          | x2.50               |
| v5      | ~480          | x3.00               |

---

## 1.4. Version Descriptions

- **v1:**  
  A basic ECS with global maps of fixed-size arrays for each component type, indexed by entity ID. Simple but inefficient in terms of cache locality and memory usage.

- **v2:**  
  Introduces sparse-set storage for components, improving add/remove operations and cache friendliness. No archetype grouping yet.

- **v3:**  
  Implements archetypes to group entities by component signatures. Uses polymorphic component arrays and enables iteration over matching archetypes, greatly improving iteration speed.

- **v4:**  
  Refines archetype management with modern C++ features (unique pointers, move semantics). Adds entity location tracking and partial support for dynamic component addition by moving data between archetypes.

- **v5:**  
  Fully typed ECS with compile-time component lists and bitmask-based archetype signatures. Implements efficient component addition/removal, type-safe APIs, and improved performance through bitwise archetype queries.

---

## 1.5. Examples and Demos

Two example tech demos are provided:

- `example/ecs` (v5): Runs with 100,000 entities managed by the ECS.  
- `example/oop`: Runs with 100,000 entities using a classic OOP approach with inheritance.

**Observations:**

- Frame times between the ECS and OOP demos are similar, with ECS showing a slight performance advantage.  
- The ECS movement calculation is approximately **3 times faster** than the OOP implementation, demonstrating the cache-friendly and efficient design of the ECS pattern.

---

This project showcases how ECS design evolves to achieve better performance, maintainability, and scalability in complex simulations.

## 1.6. License

Copyright 2025 Microsoft Corporation
Copyright © Luke Heydel All rights reserved.<br />
Licensed under the MIT License. See LICENSE in the project root for license information.