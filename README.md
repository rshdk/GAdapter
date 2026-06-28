# POADAPTER

POADAPTER is a secure, web-based client administration and task-management panel built using Flask. It allows administrators to monitor active client nodes, configure and patch new client payloads, queue tasks (such as PowerShell commands or executable files), and inspect execution responses in a centralized dashboard.

## Screenshot

![Dashboard Screenshot](screenshots/dashboard.png)

## Features

- Client Dashboard: Real-time monitoring of connected clients, showing IP addresses, geolocation resolution with flags, active status, and last seen timestamps.
- Task Management System: Send commands or file-based tasks (PowerShell scripts, executables, or DLL payloads) directly to specific clients.
- Automated Tasks (Auto Tasks): Define persistent task templates that auto-deploy to checking-in clients exactly once.
- Client Builder: Patch client binaries with custom server endpoints, client IDs, and communication intervals before wrapping them into encrypted ZIP archives.
- User and Access Controls: Security features including user authorization and admin-only settings.
- Secure WSGI Deployment: Built-in gevent-based WSGI server supporting SSL certificate auto-generation.

## Prerequisites

Ensure you have Python 3.8+ installed on your system.

## Installation

1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/your-username/POADAPTER.git
   cd POADAPTER
   ```

2. Install the required dependencies:
   ```bash
   pip install -r requirements.txt
   ```

3. (Optional) Configure environment variables:
   - SECRET_KEY: Flask secret key.
   - DB_ENGINE, DB_USERNAME, DB_PASS, DB_HOST, DB_PORT, DB_NAME: Relational database configuration (e.g., PostgreSQL or MySQL). Defaults to local SQLite if unspecified.
   - GITHUB_ID, GITHUB_SECRET: Client credentials for GitHub OAuth login integration.

## Usage

### Local Debugging

To run the application locally on HTTP (port 3000) for testing:
```bash
python runLocal.py
```
Access the application at http://localhost:3000

### Production Deployment

To run the application inside a secure WSGI container with SSL (port 443):
```bash
python runWeb.py
```
- On the first run, if certificate.crt and private.key are not present in the root directory, the server will automatically generate self-signed SSL credentials.
- Access the secure application at https://localhost:443 (or your configured domain).

## Project Structure

- apps/: The core Flask application package containing templates, static assets, databases, and routes.
  - authentication/: Handles user registration, logging in, database models, and OAuth.
  - home/: Contains dashboard view routes, client communication endpoints, task handling logic, and payload patchers.
- runLocal.py: Setup script for running the application in a local debugging environment.
- runWeb.py: Server script for running the production-ready WSGI server under HTTPS.
- screenshots/: Folder containing UI mockups and dashboards for repository demonstration.
