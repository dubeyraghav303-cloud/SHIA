import { auth0 } from "@/lib/auth0";
import DashboardClient from "@/components/DashboardClient";

export default async function Home() {
  const session = await auth0.getSession();

  if (!session) {
    return (
      <main className="min-h-screen bg-neutral-950 flex flex-col items-center justify-center text-white">
        <div className="max-w-md w-full p-8 border border-neutral-800 bg-neutral-900 rounded-lg shadow-2xl flex flex-col items-center text-center space-y-6">
          <div className="w-16 h-16 bg-blue-500/20 text-blue-500 rounded-full flex items-center justify-center">
            <svg xmlns="http://www.w3.org/2000/svg" className="h-8 w-8" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 15v2m-6 4h12a2 2 0 002-2v-6a2 2 0 00-2-2H6a2 2 0 00-2 2v6a2 2 0 002 2zm10-10V7a4 4 0 00-8 0v4h8z" />
            </svg>
          </div>
          <h1 className="text-2xl font-bold tracking-tight">SHIA Control Room</h1>
          <p className="text-neutral-400 text-sm">
            Access to the Self-Healing Infrastructure Agent telemetry is restricted. Please sign in to verify your identity.
          </p>
          <a
            href="/api/auth/login"
            className="w-full bg-blue-600 hover:bg-blue-500 text-white font-medium py-3 px-4 rounded transition-colors"
          >
            Authenticate with Auth0
          </a>
        </div>
      </main>
    );
  }

  return (
    <main className="min-h-screen bg-[#0A0A0A] text-gray-300 font-sans selection:bg-blue-500/30">
      <DashboardClient user={session.user} />
    </main>
  );
}
