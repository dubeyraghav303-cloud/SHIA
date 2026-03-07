import { NextResponse, type NextRequest } from "next/server";
import { Auth0Client } from "@auth0/nextjs-auth0/server";

export async function middleware(request: NextRequest) {
    try {
        const auth0 = new Auth0Client();
        return await auth0.middleware(request);
    } catch (error: any) {
        console.error("Auth0 Middleware Crash - Environment Variable Missing:", error);
        return new NextResponse(`SHIA Vercel Error: ${error.message || error}. Please ensure all AUTH0 env vars are added to Vercel Project Settings.`, { status: 500 });
    }
}

export const config = {
    matcher: [
        "/dashboard/:path*",
        "/api/incidents/:path*",
        "/auth/:path*"
    ]
};
