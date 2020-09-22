select count(pid)
from Pokemon p, CatchedPokemon c
where p.id = c.pid
group by p.type
order by type;
