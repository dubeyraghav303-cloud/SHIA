#!/usr/bin/env python3

import requests
import sys
import os

def check_dashboard(url):
    print(f"🔍 Pinging Vercel Dashboard at {url}...")
    try:
        # We expect a 200 (if public) or 302 (if blocked by Auth0 middleware)
        # We allow redirects to observe the Auth0 passthrough
        response = requests.get(url, timeout=5, allow_redirects=False)
        if response.status_code in [200, 302]:
            print(f"✅ Dashboard reachable (HTTP {response.status_code})")
            return True
        else:
            print(f"❌ Dashboard unreachable (HTTP {response.status_code})")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ Dashboard unreachable: {e}")
        return False

def check_supabase(url, key):
    print(f"🔍 Pinging Supabase REST API at {url}...")
    headers = {
        'apikey': key,
        'Authorization': f'Bearer {key}'
    }
    try:
        response = requests.get(f"{url}/rest/v1/incidents?limit=1", headers=headers, timeout=5)
        if response.status_code == 200:
            print("✅ Supabase REST API reachable and responding")
            return True
        else:
            print(f"❌ Supabase REST API error (HTTP {response.status_code})")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ Supabase REST API unreachable: {e}")
        return False

if __name__ == "__main__":
    print("===========================================")
    print("   SHIA Remote Health Check Utility        ")
    print("===========================================")
    
    dashboard_url = os.environ.get("VERCEL_URL", "http://localhost:3000")
    supabase_url = os.environ.get("SUPABASE_URL", "")
    supabase_key = os.environ.get("SUPABASE_KEY", "")
    
    if not supabase_url or not supabase_key:
        print("⚠️ Warning: SUPABASE_URL or SUPABASE_KEY environment variables not set.")
        print("Will attempt to extract from core/config/shia_config.json if possible (simulated for dummy fallback).\n")
        # For the sake of this test script, we assume they are passed via ENV
        sys.exit(1)

    dash_ok = check_dashboard(dashboard_url + "/dashboard")
    db_ok = check_supabase(supabase_url, supabase_key)
    
    print("===========================================")
    if dash_ok and db_ok:
        print("🟢 Remote Health Check PASSED. Multi-Cloud links are active.")
        sys.exit(0)
    else:
        print("🔴 Remote Health Check FAILED. Please review routing configurations.")
        sys.exit(1)
