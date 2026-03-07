import type { NextRequest } from "next/server";
import { Auth0Client } from "@auth0/nextjs-auth0/edge";

const auth0 = new Auth0Client();

export async function middleware(request: NextRequest) {
    return await auth0.middleware(request);
}

export const config = {
    matcher: [
        "/dashboard/:path*",
        "/api/incidents/:path*",
        "/auth/:path*"
    ]
};
