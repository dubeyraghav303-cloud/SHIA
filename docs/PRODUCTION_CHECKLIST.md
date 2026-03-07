# Phase 7: Production Integration Checklist

Before pointing your live web traffic or infrastructure alerts to SHIA, you must finalize the multi-cloud routing configurations so that all components (C++ Engine, Supabase, Vercel Dashboard, Auth0) correctly map to each other in a production environment.

## 1. Vercel Preparation
When you deploy `dashboard/` to Vercel, it assigns a dynamic domain (e.g., `shia-dashboard-xyz.vercel.app`).
1. [ ] Log into Vercel.
2. [ ] Identify your exact production URL.
3. [ ] Set `AUTH0_BASE_URL` in Vercel to `https://your-vercel-domain.vercel.app`
4. [ ] Ensure `NEXT_PUBLIC_SUPABASE_URL` is set to your production Supabase database.

## 2. Auth0 Callbacks
Auth0 strictly blocks redirects to unauthorized domains. You MUST tell Auth0 about your Vercel deployment.
1. [ ] Log into the Auth0 Management Dashboard.
2. [ ] Go to Applications -> Your Application.
3. [ ] Update **Allowed Callback URLs**: Add `https://your-vercel-domain.vercel.app/api/auth/callback`
4. [ ] Update **Allowed Logout URLs**: Add `https://your-vercel-domain.vercel.app/`
5. [ ] Update **Allowed Web Origins**: Add `https://your-vercel-domain.vercel.app`
6. [ ] Save Changes.

## 3. Remote Health Check
Before spinning up the C++ `shia_engine` on your remote Linux boxes, execute the health check from the box:
```bash
# Setup environment variables on the remote box
export VERCEL_URL="https://your-vercel-domain.vercel.app"
export SUPABASE_URL="https://your-production-db.supabase.co"
export SUPABASE_KEY="your-anon-key"

# Run the health check check
python3 tests/remote_health_check.py
```
*Wait for the `🟢 Remote Health Check PASSED` output.*

## 4. Launching the Engine
1. [ ] Ensure your `core/config/shia_config.json` uses the production Supabase URLs.
2. [ ] Launch the engine via `systemd` or standard `./shia_engine`.
3. [ ] Monitor logs to confirm `Telemetry sync enabled to https://your-production-db.supabase.co`.
