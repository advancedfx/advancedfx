namespace AfxGui {

class Globals
{
    internal static bool AutoStartAfxHookGoldSrc { get { return m_AutoStartAfxHookGoldSrc; } set { m_AutoStartAfxHookGoldSrc = value; } }
    internal static bool AutoStartCsgo { get { return m_AutoStartCsgo; } set { m_AutoStartCsgo = value; } }
    internal static bool AutoStartCustomLoader { get { return m_AutoStartCustomLoader; } set { m_AutoStartCustomLoader = value; } }
    internal static bool NoGui { get { return m_NoGui; } set { m_NoGui = value; } }

    static bool m_AutoStartAfxHookGoldSrc;
    static bool m_AutoStartCsgo;
    static bool m_AutoStartCustomLoader;
    static bool m_NoGui;
}

class GlobalConfig
{
    internal static Config Instance { get { return m_Instance; } set { m_Instance = value; } }
    static Config m_Instance;
}


class GlobalUpdateCheck
{
    internal static UpdateCheck Instance { get { return m_Instance; } set { m_Instance = value; } }
    static UpdateCheck m_Instance;
}

} //namespace AfxGui {
