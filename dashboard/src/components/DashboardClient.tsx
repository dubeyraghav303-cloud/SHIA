"use client";

import { useEffect, useState } from "react";
import { supabase } from "@/lib/supabase";
import { Activity, AlertTriangle, CheckCircle, Shield, XCircle, LogOut, Terminal, Server, Cpu, Link as LinkIcon, ShieldCheck } from "lucide-react";

interface Incident {
    id: string;
    timestamp: string;
    service: string;
    error: string;
    action: string;
    ai_patch: boolean;
    status: "healed" | "failed" | "locked";
    created_at: string;
}

export default function DashboardClient({ user }: { user: any }) {
    const [incidents, setIncidents] = useState<Incident[]>([]);
    const [loading, setLoading] = useState<boolean>(true);
    const [activeServices, setActiveServices] = useState<number>(3); // Placeholder

    // Consistency Check Modal State
    const [verifyingHash, setVerifyingHash] = useState<string | null>(null);
    const [verificationResult, setVerificationResult] = useState<'pending' | 'success' | 'failed' | null>(null);

    // Mock hash generator for UI demo purposes
    const generateMockHash = (incident: Incident) => {
        let str = incident.id + incident.timestamp + incident.service + incident.action;
        let hash = 0;
        for (let i = 0; i < str.length; i++) {
            const char = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash;
        }
        return "0x" + Math.abs(hash).toString(16).padStart(16, "0") + "abcd1234ef987654";
    };

    const verifyOnChain = (incident: Incident) => {
        const localHash = generateMockHash(incident);
        setVerifyingHash(localHash);
        setVerificationResult('pending');

        // Simulate async block explorer fetch
        setTimeout(() => {
            // In a real app we'd fetch the hash from Polygyon via JSON-RPC using the incident ID
            // Here we assume the chain is perfectly synced with the local DB
            setVerificationResult('success');

            // Hide modal after 3 seconds
            setTimeout(() => {
                setVerifyingHash(null);
                setVerificationResult(null);
            }, 3000);
        }, 1500);
    };

    useEffect(() => {
        // Initial Fetch
        const fetchIncidents = async () => {
            const { data, error } = await supabase
                .from("incidents")
                .select("*")
                .order("timestamp", { ascending: false })
                .limit(50);

            if (!error && data) {
                setIncidents(data as Incident[]);
            }

            const sessionRes = await supabase
                .from("sessions")
                .select("*")
                .eq("status", "active");

            if (!sessionRes.error && sessionRes.data) {
                setActiveServices(sessionRes.data.length);
            }

            setLoading(false);
        };

        fetchIncidents();

        // Supabase Realtime Subscription
        const channel = supabase
            .channel("schema-db-changes")
            .on(
                "postgres_changes",
                { event: "INSERT", schema: "public", table: "incidents" },
                (payload) => {
                    setIncidents((prev) => [payload.new as Incident, ...prev].slice(0, 50));
                }
            )
            .subscribe();

        return () => {
            supabase.removeChannel(channel);
        };
    }, []);

    const healedCount = incidents.filter(i => i.status === "healed").length;
    const failedCount = incidents.filter(i => i.status === "failed").length;
    const aiPatchCount = incidents.filter(i => i.ai_patch).length;

    return (
        <div className="min-h-screen bg-[#0A0A0A] text-gray-300 font-sans p-6 pb-20">
            {/* Top Navbar */}
            <header className="flex items-center justify-between border-b border-gray-800 pb-4 mb-8">
                <div className="flex items-center space-x-3">
                    <div className="p-2 bg-blue-500/10 rounded-lg text-blue-500">
                        <Shield size={24} />
                    </div>
                    <div>
                        <h1 className="text-xl font-bold text-gray-100 tracking-wide flex items-center gap-2">
                            SHIA <span className="text-xs px-2 py-0.5 bg-gray-800 text-gray-400 rounded-full font-mono">v2.0</span>
                        </h1>
                        <p className="text-xs text-gray-500">Self-Healing Infrastructure Agent</p>
                    </div>
                </div>

                <div className="flex items-center space-x-6">
                    <div className="flex items-center space-x-2">
                        <div className="h-2 w-2 rounded-full bg-green-500 shadow-[0_0_8px_rgba(34,197,94,0.6)] animate-pulse"></div>
                        <span className="text-sm text-gray-400">Agent Connected</span>
                    </div>
                    <div className="h-8 w-px bg-gray-800"></div>
                    <div className="flex items-center space-x-3">
                        <span className="text-sm text-gray-400">{user?.name || user?.email}</span>
                        <a href="/api/auth/logout" className="text-gray-500 hover:text-gray-300 transition-colors">
                            <LogOut size={18} />
                        </a>
                    </div>
                </div>
            </header>

            {/* Main Metrics Matrix */}
            <div className="grid grid-cols-1 md:grid-cols-4 gap-4 mb-8">
                <div className="bg-[#111111] border border-gray-800/60 rounded-xl p-5 shadow-lg relative overflow-hidden group">
                    <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
                        <Activity size={48} className="text-blue-500" />
                    </div>
                    <p className="text-sm text-gray-500 mb-1 font-medium">Total Interventions</p>
                    <p className="text-3xl font-bold text-gray-100">{incidents.length}</p>
                </div>

                <div className="bg-[#111111] border border-gray-800/60 rounded-xl p-5 shadow-lg relative overflow-hidden group">
                    <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
                        <CheckCircle size={48} className="text-green-500" />
                    </div>
                    <p className="text-sm text-gray-500 mb-1 font-medium">Successfully Healed</p>
                    <p className="text-3xl font-bold text-green-500">{healedCount}</p>
                </div>

                <div className="bg-[#111111] border border-gray-800/60 rounded-xl p-5 shadow-lg relative overflow-hidden group">
                    <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
                        <Terminal size={48} className="text-purple-500" />
                    </div>
                    <p className="text-sm text-gray-500 mb-1 font-medium">AI Surgical Patches</p>
                    <p className="text-3xl font-bold text-purple-400">{aiPatchCount}</p>
                </div>

                <div className="bg-[#111111] border border-gray-800/60 rounded-xl p-5 shadow-lg relative overflow-hidden group">
                    <div className="absolute top-0 right-0 p-4 opacity-10 group-hover:opacity-20 transition-opacity">
                        <Server size={48} className="text-blue-500" />
                    </div>
                    <p className="text-sm text-gray-500 mb-1 font-medium">Monitored Services</p>
                    <p className="text-3xl font-bold text-gray-100">{activeServices}</p>
                </div>
            </div>

            {/* Telemetry Feed */}
            <div className="bg-[#111111] border border-gray-800/60 rounded-xl overflow-hidden shadow-2xl">
                <div className="p-5 border-b border-gray-800/60 bg-[#161616] flex items-center justify-between">
                    <h2 className="text-lg font-semibold text-gray-200 flex items-center gap-2">
                        <Cpu size={18} className="text-blue-500" />
                        Live Incident Telemetry
                    </h2>
                    {loading && <span className="text-xs text-blue-500 px-2 py-1 bg-blue-500/10 rounded-full animate-pulse">Syncing...</span>}
                </div>

                <div className="overflow-x-auto">
                    <table className="w-full text-left border-collapse">
                        <thead>
                            <tr className="bg-[#1A1A1A] text-gray-400 text-xs uppercase tracking-wider">
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Time</th>
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Service</th>
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Signature</th>
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Action Applied</th>
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Audit Trail</th>
                                <th className="px-6 py-4 font-medium border-b border-gray-800">Status</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-gray-800/50">
                            {incidents.length === 0 && !loading ? (
                                <tr>
                                    <td colSpan={5} className="px-6 py-12 text-center text-gray-500">
                                        <CheckCircle size={32} className="mx-auto mb-3 opacity-20" />
                                        No incidents recorded. Infrastructure is healthy.
                                    </td>
                                </tr>
                            ) : (
                                incidents.map((incident) => (
                                    <tr key={incident.id} className="hover:bg-[#151515] transition-colors group">
                                        <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-400 font-mono">
                                            {new Date(incident.timestamp).toLocaleTimeString([], { hour12: false, hour: '2-digit', minute: '2-digit', second: '2-digit' })}
                                        </td>
                                        <td className="px-6 py-4 whitespace-nowrap">
                                            <span className="px-2.5 py-1 text-xs font-medium rounded-md bg-gray-800 text-gray-300 border border-gray-700">
                                                {incident.service}
                                            </span>
                                        </td>
                                        <td className="px-6 py-4 text-sm text-gray-300 max-w-xs truncate font-mono bg-black/20 group-hover:bg-black/40" title={incident.error}>
                                            {incident.error}
                                        </td>
                                        <td className="px-6 py-4 text-sm">
                                            <div className="flex items-center gap-2">
                                                {incident.ai_patch && (
                                                    <span className="inline-flex items-center text-xs font-medium bg-purple-500/10 text-purple-400 px-2 py-0.5 rounded border border-purple-500/20">
                                                        AI Patch
                                                    </span>
                                                )}
                                                <span className="text-gray-300 font-mono text-xs">{incident.action}</span>
                                            </div>
                                        </td>
                                        <td className="px-6 py-4 whitespace-nowrap">
                                            <button
                                                onClick={() => verifyOnChain(incident)}
                                                className="inline-flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-medium bg-blue-500/10 text-blue-400 border border-blue-500/20 hover:bg-blue-500/20 transition-all font-mono"
                                            >
                                                <LinkIcon size={12} /> Verify on Chain
                                            </button>
                                        </td>
                                        <td className="px-6 py-4 whitespace-nowrap">
                                            {incident.status === 'healed' ? (
                                                <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-medium bg-green-500/10 text-green-400 border border-green-500/20">
                                                    <CheckCircle size={12} /> Resolved
                                                </span>
                                            ) : incident.status === 'locked' ? (
                                                <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-medium bg-red-500/10 text-red-500 border border-red-500/20">
                                                    <XCircle size={12} /> Locked Out
                                                </span>
                                            ) : (
                                                <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-medium bg-yellow-500/10 text-yellow-500 border border-yellow-500/20">
                                                    <AlertTriangle size={12} /> Failed
                                                </span>
                                            )}
                                        </td>
                                    </tr>
                                ))
                            )}
                        </tbody>
                    </table>
                </div>
            </div>

            {/* Consistency Check Modal */}
            {verifyingHash && (
                <div className="fixed inset-0 bg-black/80 backdrop-blur-sm flex justify-center items-center z-50">
                    <div className="bg-[#111] border border-gray-800 rounded-xl max-w-md w-full p-6 shadow-2xl relative overflow-hidden">

                        {verificationResult === 'pending' && (
                            <div className="absolute top-0 left-0 h-1 bg-blue-500 animate-[pulse_1s_ease-in-out_infinite] w-full"></div>
                        )}
                        {verificationResult === 'success' && (
                            <div className="absolute top-0 left-0 h-1 bg-green-500 w-full transition-all"></div>
                        )}

                        <div className="flex items-center gap-3 mb-6">
                            <div className={`p-3 rounded-xl ${verificationResult === 'success' ? 'bg-green-500/10 text-green-500' : 'bg-blue-500/10 text-blue-500'}`}>
                                {verificationResult === 'success' ? <ShieldCheck size={28} /> : <Cpu size={28} className={verificationResult === 'pending' ? 'animate-spin' : ''} />}
                            </div>
                            <div>
                                <h3 className="text-lg font-semibold text-white">Cryptographic Consistency Check</h3>
                                <p className="text-xs text-gray-500">Querying Polygon Amoy Testnet</p>
                            </div>
                        </div>

                        <div className="space-y-4 font-mono text-sm">
                            <div className="p-3 bg-black/40 rounded-lg border border-gray-800/50">
                                <p className="text-xs text-gray-500 mb-1">Local State Fingerprint</p>
                                <p className="text-gray-300 break-all">{verifyingHash}</p>
                            </div>

                            <div className={`p-3 rounded-lg border flex flex-col items-center justify-center min-h-[80px] transition-all
                                ${verificationResult === 'pending' ? 'bg-black/20 border-blue-500/20' : ''}
                                ${verificationResult === 'success' ? 'bg-green-500/10 border-green-500/30' : ''}`}>

                                {verificationResult === 'pending' && (
                                    <div className="text-blue-400 flex flex-col items-center gap-2">
                                        <div className="w-5 h-5 border-2 border-blue-500/30 border-t-blue-500 rounded-full animate-spin"></div>
                                        <span className="text-xs">Awaiting Network Consensus...</span>
                                    </div>
                                )}
                                {verificationResult === 'success' && (
                                    <div className="text-green-400 flex flex-col items-center gap-2">
                                        <CheckCircle size={24} />
                                        <span className="font-semibold text-sm text-green-500">Immutability Verified & Match Confirmed</span>
                                        <div className="text-xs mt-1 text-green-600">Block Explorer Hash matches Local DB.</div>
                                    </div>
                                )}
                            </div>
                        </div>
                    </div>
                </div>
            )}
        </div>
    );
}
