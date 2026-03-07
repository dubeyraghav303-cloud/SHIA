# SHIA Validation & Testing Guide

This document outlines the standard operating procedures for verifying the Auth0, Supabase, and AI Repair integrations before moving to production deployments. Use this checklist to certify the pipeline.

## 1. Auth0 Middleware Audit

The Next.js dashboard is protected by an Auth0 Edge Middleware proxy. It is configured to block any unauthenticated access to the application data, including APIs and Dashboard routes.

**Test Case:**
Run the following `curl` command to verify that an unauthenticated request to the dashboard or an API route is intercepted and strictly redirected (HTTP 302) to the Auth0 login flow:
```bash
# Verify the Next.js server is running first (npm run dev in dashboard/)
curl -I http://localhost:3000/dashboard
curl -I http://localhost:3000/api/incidents
```
**Expected Outcome:**
You should see a `HTTP/1.1 302 Found` header, with a `Location: /api/auth/login` parameter.

## 2. Supabase Connectivity Heartbeat

To ensure the Supabase REST API and configurations are functional, a standalone bash script interacts directly with the `incidents` table.

**Test Case:**
```bash
./tests/check_connectivity.sh
```
**Expected Outcome:**
1. A `HTTP 201` post request to the Supabase endpoint.
2. A successful printout: `✅ Success! Supabase telemetry connection verified`.
3. An output verifying the table can be read via the Anon key.

## 3. Chaos Monkey (Mock Failure Injection)

We verify the SHIA Engine's AI Fallback loop by artificially causing crashes within monitored simulated log environments.

### Scenario A: Simple Crash recovery
Injects a basic SegFault mimicking a process death.
```bash
./build/chaos_monkey scenario_a
```
**Expected Logs (SHIA Engine):**
- `[SHIA-Detector] ⚠️ ALERT TRIGGERED ⚠️`
- `[SHIA-Recoverer] Executing Action: RESTART_SERVICE`
- `[SHIA-Telemetry] Successfully synced incident (healed) to dashboard.`

### Scenario B: AI Fallback Verification
Poisons a simulated configuration file to bypass generic recovery (Restart) and force SHIA to consult Gemini AI.
```bash
./build/chaos_monkey scenario_b
```
**Expected Logs (SHIA Engine):**
- `[SHIA-Recoverer] ❌ PRIMARY ACTION FAILED.`
- `[SHIA-Recoverer] 🤖 Initiating AI Fallback Fix...`
- `[SHIA-AI-Bridge] Requesting fallback fix from Gemini API...`
- `[SHIA-Telemetry] Successfully synced incident (healed) to dashboard.`

## 4. Telemetry Payload & Session Integrity

Verify that the C++ Engine securely passes contextual state alongside every REST POST to Supabase.

1. Tail the backend SHIA engine logs. Ensure `Active Session ID` is generated on boot.
2. In the Next.js UI component, monitor the **Live Incident Telemetry** table.
3. Validate that a single SHIA Engine instance uses the *exact same* `session_id` UUID for its entire lifespan across multiple triggered events.
