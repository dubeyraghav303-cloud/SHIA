# SHIA: Production Deployment Guide

Deploying the complete SHIA architecture requires separating the C++ binary runtime from the Next.js Web Dashboard.

## 1. Deploying the Next.js Dashboard (Vercel)

The easiest way to host the Next.js frontend is via Vercel.

**Steps:**
1. Push your complete `SHCR` repository to GitHub.
2. Log into [Vercel](https://vercel.com/) and click **Add New Project**.
3. Import your `SHCR` repository.
4. **Important Framework Preset:** Ensure the Root Directory is set to `dashboard` (Vercel will detect it as Next.js automatically).
5. **Environment Variables:** Add all the variables from your `dashboard/.env.local` to the Vercel project settings:
   - `AUTH0_SECRET`
   - `AUTH0_BASE_URL` *(Change this to your actual production Vercel URL, e.g., `https://shia-dashboard.vercel.app`)*
   - `AUTH0_ISSUER_BASE_URL`
   - `AUTH0_CLIENT_ID`
   - `AUTH0_CLIENT_SECRET`
   - `NEXT_PUBLIC_SUPABASE_URL`
   - `NEXT_PUBLIC_SUPABASE_ANON_KEY`
6. Click **Deploy**. Vercel will build and host your Telemetry Control Room globally.

> **Auth0 Reminder:** Remember to update your Auth0 Application settings (Allowed Callback URLs, Allowed Logout URLs, Allowed Web Origins) to include your new production Vercel domain!

---

## 2. Deploying the SHIA C++ Engine (Linux Server)

The SHIA Engine runs natively on the host it is protecting (e.g., an Ubuntu AWS EC2 instance, DigitalOcean Droplet, etc.).

**Prerequisites on the Production Server:**
```bash
sudo apt-update
sudo apt install build-essential cmake libcurl4-openssl-dev
```

**Compilation:**
Clone your repository onto the production Linux server. Then, compile it specific to the Linux architecture.
```bash
cd SHCR/build
rm -rf *
cmake ..
make
```

### Running as a Background Daemon (Systemd)

You do not want to run SHIA in a standard terminal session that closes when you disconnect. Instead, create a `systemd` service to keep it alive forever and restart it on reboot.

1. Create a service file:
```bash
sudo nano /etc/systemd/system/shia-engine.service
```

2. Add the following configuration (Adjust `/path/to/SHCR` to your actual clone directory):
```ini
[Unit]
Description=SHIA Self-Healing Infrastructure Agent
After=network.target

[Service]
Type=simple
User=root
# We run as root or a highly privileged user so SHIA can restart services (like Nginx) and flush caches
WorkingDirectory=/path/to/SHCR
ExecStart=/path/to/SHCR/app/SHIA.app/Contents/MacOS/shia_engine
Restart=on-failure
RestartSec=5

# To ensure the engine picks up the right config
Environment="LD_LIBRARY_PATH=/usr/local/lib"

[Install]
WantedBy=multi-user.target
```

3. Enable and start the daemon:
```bash
sudo systemctl daemon-reload
sudo systemctl enable shia-engine
sudo systemctl start shia-engine
```

4. Check the logs to verify it connected to Supabase:
```bash
sudo journalctl -u shia-engine -f
```

---

## 3. Database & Smart Contract Requirements

- **Supabase:** Ensure your `incidents` table has Row Level Security (RLS) configured appropriately so malicious actors cannot spam dummy telemetry.
- **Smart Contract:** The provided Solidity contract `contracts/SHIAAudit.sol` must be compiled and deployed to a mainnet (e.g., Polygon) using Hardhat/Truffle or Remix IDE. Once deployed, you will update the mock C++ `BlockchainBridge` to make real JSON-RPC calls via `libcurl` to your deployed contract address via Alchemy or Infura!
