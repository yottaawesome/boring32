// https://learn.microsoft.com/en-us/windows-server/security/windows-authentication/security-support-provider-interface-architecture
export module boring32.sspi;
export import :credential;
export import :securitycontext;
export import :types;
export import :contextbuffer;
export import :sizedcontextbuffer;
export import :buffertype;
export import boring32.sspi.provider.credential;
export import boring32.sspi.provider.digest;
export import boring32.sspi.provider.negotiate;
export import boring32.sspi.provider.ntlm;
export import boring32.sspi.provider.kerberos;
export import boring32.sspi.provider.pku2u;
export import boring32.sspi.provider.schannel;
