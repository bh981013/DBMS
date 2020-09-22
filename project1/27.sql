select t.name, max(level)
from CatchedPokemon c, Trainer t
where c.owner_id = t.id
group by t.id
having count(c.id) >= 4
order by t.name;