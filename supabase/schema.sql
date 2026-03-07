-- SHIA Supabase Schema
-- Run this in the Supabase SQL Editor to set up the DB

-- 1. Create incidents table for telemetry
CREATE TABLE incidents (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    session_id TEXT,
    timestamp TIMESTAMPTZ NOT NULL,
    service TEXT NOT NULL,
    error TEXT NOT NULL,
    action TEXT NOT NULL,
    ai_patch BOOLEAN DEFAULT false,
    status TEXT NOT NULL, -- 'healed', 'failed', 'locked'
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Enable Realtime for the SRE Dashboard
ALTER TABLE incidents REPLICA IDENTITY FULL;

-- 2. Create sessions table for engine state
CREATE TABLE sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    engine_id TEXT UNIQUE NOT NULL,
    status TEXT NOT NULL, -- 'active', 'offline', 'error'
    active_monitors INTEGER DEFAULT 0,
    last_ping TIMESTAMPTZ DEFAULT NOW()
);

-- Enable Realtime for sessions
ALTER TABLE sessions REPLICA IDENTITY FULL;

-- 3. Row Level Security (RLS) configuration

-- Enable RLS
ALTER TABLE incidents ENABLE ROW LEVEL SECURITY;
ALTER TABLE sessions ENABLE ROW LEVEL SECURITY;

-- Create policy to allow anonymous POST requests from the C++ agent
CREATE POLICY "Allow anonymous insert from agent" ON incidents FOR INSERT TO anon WITH CHECK (true);
CREATE POLICY "Allow anonymous update from agent" ON sessions FOR ALL TO anon USING (true) WITH CHECK (true);

-- Create policy to allow authenticated users (via Auth0 JWT or Supabase Dashboard) to select
CREATE POLICY "Allow authenticated read" ON incidents FOR SELECT TO authenticated USING (true);
CREATE POLICY "Allow authenticated read" ON sessions FOR SELECT TO authenticated USING (true);
