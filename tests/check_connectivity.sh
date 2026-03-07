#!/usr/bin/env bash

# check_connectivity.sh
# Tests connection to the Supabase REST API telemetry endpoints.

set -e

source core/config/shia_config.json 2>/dev/null || true
# Fallback hardcoded if jq/parsing is complex, but let's parse using grep
URL=$(grep -o '"supabase_url": *"[^"]*"' core/config/shia_config.json | grep -o '"[^"]*"$' | sed 's/"//g')
KEY=$(grep -o '"supabase_key": *"[^"]*"' core/config/shia_config.json | grep -o '"[^"]*"$' | sed 's/"//g')

if [ -z "$URL" ] || [ -z "$KEY" ]; then
    echo "❌ Error: Could not find supabase_url or supabase_key in core/config/shia_config.json"
    exit 1
fi

echo "🔍 Pinging Supabase (Heartbeat Test)..."
echo "URL: $URL/rest/v1/incidents"

HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$URL/rest/v1/incidents" \
  -H "apikey: $KEY" \
  -H "Authorization: Bearer $KEY" \
  -H "Content-Type: application/json" \
  -H "Prefer: return=minimal" \
  -d '{
    "session_id": "test-session-1234",
    "timestamp": "'$(date -u +"%Y-%m-%dT%H:%M:%SZ")'",
    "service": "test_service",
    "error": "connectivity_check",
    "action": "HEARTBEAT",
    "ai_patch": false,
    "status": "healed"
  }')

if [ "$HTTP_CODE" == "201" ] || [ "$HTTP_CODE" == "200" ]; then
    echo "✅ Success! Supabase telemetry connection verified (HTTP $HTTP_CODE)."
else
    echo "❌ Failure! Failed to connect or insert row (HTTP $HTTP_CODE)."
    exit 1
fi

# Try to read back
echo "🔍 Attempting to sweep test entry..."
READ_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$URL/rest/v1/incidents?service=eq.test_service&select=*" \
  -H "apikey: $KEY" \
  -H "Authorization: Bearer $KEY")

if [ "$READ_CODE" == "200" ]; then
    echo "✅ Success! Able to query incidents directly from the source pipeline."
else
    echo "❌ Failure! Could not read incidents table (HTTP $READ_CODE)."
fi
