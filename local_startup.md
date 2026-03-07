# SHIA: Local Startup Guide

This guide covers everything you need to start the SHIA Engine and the Next.js Dashboard locally on your macOS machine for end-to-end testing.

## Prerequisites

Before starting, ensure you have the following installed on your machine:
- **C++ Build Tools:** `cmake` (>= 3.15), `g++` (AppleClang or GCC), `make`
- **Libraries:** `libcurl` (usually pre-installed on macOS, or `brew install curl`)
- **Node.js:** `node` (>= 18.x) and `npm`
- **External Services Accounts:**
  - Supabase (Database & Realtime)
  - Auth0 (Authentication Edge Middleware)
  - Gemini API Key (AI Repair Fallback)

---

## 1. Environment Variable Configuration

You need to establish the connections for both the C++ Engine and the Next.js Frontend.

### A. Next.js Dashboard (`dashboard/.env.local`)
Create a file named `.env.local` inside the `/dashboard` directory:
```bash
cd dashboard
touch .env.local
```
Add the following variables and replace the placeholders:
```env
# Auth0 Configuration
AUTH0_SECRET='generate_a_long_random_string_like_32_characters_long'
AUTH0_BASE_URL='http://localhost:3000'
AUTH0_ISSUER_BASE_URL='https://YOUR_AUTH0_DOMAIN'
AUTH0_CLIENT_ID='YOUR_AUTH0_CLIENT_ID'
AUTH0_CLIENT_SECRET='YOUR_AUTH0_CLIENT_SECRET'

# Supabase Configuration
NEXT_PUBLIC_SUPABASE_URL='https://YOUR_SUPABASE_ID.supabase.co'
NEXT_PUBLIC_SUPABASE_ANON_KEY='YOUR_SUPABASE_ANON_KEY'
```

### B. SHIA C++ Engine (`core/config/shia_config.json`)
The C++ engine expects a JSON configuration to load its rules, parameters, and Supabase credentials. 
Check `core/config/shia_config.json` and ensure these fields match your Supabase project:
```json
{
  "supabase_url": "https://YOUR_SUPABASE_ID.supabase.co",
  "supabase_key": "YOUR_SUPABASE_ANON_KEY",
  "...": "..."
}
```

---

## 2. Compiling the SHIA Engine

Use CMake to generate the build files and compile the engine.

```bash
# From the root directory (SHCR pre/)
cd build

# Clean any old builds (optional but recommended)
rm -rf *

# Run CMake and Compile
cmake ..
make

# Also compile the Chaos Monkey testing tool
g++ ../tests/chaos_monkey.cpp -o chaos_monkey

# Go back to the root directory
cd ..
```

---

## 3. Starting the Dashboard

Launch the Next.js telemetry interface.

```bash
# Open a new terminal window
cd dashboard

# Install NPM dependencies
npm install

# Start the development server
npm run dev
```

The dashboard will be available at `http://localhost:3000`. You will be prompted to log in via Auth0 before you can view the `Live Incident Telemetry`.

---

## 4. Launching the Engine & Testing

With the dashboard running, start the C++ engine to begin monitoring your dummy log files.

```bash
# Back in your primary terminal window (root directory)
./app/SHIA.app/Contents/MacOS/shia_engine
```

The engine will print its generated Session UUID and declare that it is tracking `/tmp/test_syslog.log`.

**Inject a Failure:**
Open a third terminal window and run the Chaos Monkey to simulate a real-world server crash.

```bash
# From the root directory
./build/chaos_monkey scenario_b
```

**Verification:**
Look at the SHIA engine terminal to observe the AI Patching and the Supabase upload. Then, check the Next.js Dashboard to see the new incident appear via Realtime WebSockets, and verify its Blockchain Immutability!
